/******************************************************************************
The MIT License(MIT)
https://github.com/adarwoo/cyclo

Copyright(c) 2021 Guillaume ARRECKX - software@arreckx.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/
#ifndef ui_controller_h_included
#define ui_controller_h_included

#include <boost/sml.hpp>

#include <logger.h>

#include "ui_model.hpp"
#include "ui_view.hpp"


namespace sml = boost::sml;

// Create those simple events
struct up
{};
struct down
{};
struct push
{};
struct usb_on
{};
struct usb_off
{};
struct splash_timeout
{};
struct pgm_stopped
{};

namespace
{
   ///< Dummy event to all processing forks in SM
   struct next
   {};

   ///< Marker to force the SM to evaluate 'next'
   bool controller_process_next{ false };
}  // namespace

///< Like process but allow re-entry
template<class TSM, class TEvent>
void process_event( TSM &sm, TEvent &&evt )
{
   LOG_HEADER( "sm" );

   sm.process_event( evt );

   while ( controller_process_next )
   {
      controller_process_next = false;
      sm.process_event( next{} );
   }
};

/** Set the next flag so the process event keeps pumping the SM */
inline auto call_next = []() { controller_process_next = true; };

/*
 * Common guards
 */
struct walkman
{
   auto operator()() const noexcept
   {
      using namespace sml;

      auto is_running = []( UIModel &m ) {
         return m.get_state() == UIModel::program_state_t::running;
      };
      auto is_stopped = []( UIModel &m ) {
         return m.get_state() == UIModel::program_state_t::stopped;
      };

      return make_transition_table(
         *"select"_s + sml::on_entry<_> / call_next,
         "select"_s + event<next>[ is_running ] = "do_pause"_s,
         "select"_s + event<next>[ is_stopped ] = "do_play"_s,
         "select"_s + event<next>               = "do_play_or_stop"_s

         ,
         "do_play"_s + sml::on_entry<_> / []( UIView &v ) { v.draw_walkman( 1 ); },
         "do_play"_s
            + event<push> /
                 []( UIModel &m ) {
                    // Load the program
                    m.load_command();

                    // Start the sequencer
                    fx::publish( msg::StartProgram{ true } );
                    m.set_state( UIModel::program_state_t::running );
                 } = "do_pause"_s

         ,
         "do_pause"_s + sml::on_entry<_> / []( UIView &v ) { v.draw_walkman( 1 ); },
         "do_pause"_s
            + event<push> /
                 []( UIModel &m ) {
                    fx::publish( msg::StopProgram{} );
                    m.set_state( UIModel::program_state_t::paused );
                 } = "do_play_or_stop"_s,
         "do_pause"_s + event<up>[ is_running ], 
         "do_pause"_s + event<pgm_stopped> = "do_play"_s,

         "do_play_or_stop"_s + sml::on_entry<_> / []( UIView &v ) { v.draw_walkman( 1 ); },
         "do_play_or_stop"_s
            + event<push> /
                 []( UIModel &m ) {
                    fx::publish( msg::StartProgram{ false } );
                    m.set_state( UIModel::program_state_t::running );
                 }                         = "do_pause"_s,
         "do_play_or_stop"_s + event<down> = "do_stop"_s

         ,
         "do_stop"_s + sml::on_entry<_> / []( UIView &v ) { v.draw_walkman( 2 ); },
         "do_stop"_s + event<up> = "do_play_or_stop"_s,
         "do_stop"_s
            + event<push> /
                 []( UIModel &m ) {
                    fx::publish( msg::StopProgram{} );
                    m.set_state( UIModel::program_state_t::stopped );
                 } = "do_play"_s );
   }
};

struct program_setup
{
   auto operator()() const noexcept
   {
      using namespace sml;

      auto on_seconds_top     = []( UIModel &m ) { return m.on_sec == 59; };
      auto on_seconds_bottom  = []( UIModel &m ) { return m.on_sec == 0; };
      auto on_minutes_top     = []( UIModel &m ) { return m.on_min == 99; };
      auto on_minutes_bottom  = []( UIModel &m ) { return m.on_min == 0; };
      auto off_seconds_top    = []( UIModel &m ) { return m.off_sec == 99; };
      auto off_seconds_bottom = []( UIModel &m ) { return m.off_sec == 0; };
      auto off_minutes_top    = []( UIModel &m ) { return m.off_min == 59; };
      auto off_minutes_bottom = []( UIModel &m ) { return m.off_min == 0; };

      return make_transition_table(
         *"setup_on_min"_s + event<up>[ on_minutes_top ],
         "setup_on_min"_s
            + event<up> /
                 []( UIModel &m, UIView &v ) {
                    ++m.on_min;
                    v.manual_program_draw_digit( UIView::next_highlight, 0, 0 );
                 },
         "setup_on_min"_s + event<down>[ on_minutes_bottom ],
         "setup_on_min"_s
            + event<down> /
                 []( UIModel &m, UIView &v ) {
                    --m.on_min;
                    v.manual_program_draw_digit( UIView::next_highlight, 0, 0 );
                 },
         "setup_on_min"_s
            + event<push> /
                 []( UIView &v ) {
                    v.manual_program_draw_digit( UIView::back_to_normal, 0, 0 );
                    v.manual_program_draw_digit( UIView::first_highlight, 0, 1 );
                 } = "setup_on_sec"_s

         ,
         "setup_on_sec"_s + event<up>[ on_seconds_top ],
         "setup_on_sec"_s
            + event<up> /
                 []( UIModel &m, UIView &v ) {
                    ++m.on_sec;
                    v.manual_program_draw_digit( UIView::next_highlight, 0, 1 );
                 },
         "setup_on_sec"_s + event<down>[ on_seconds_bottom ],
         "setup_on_sec"_s
            + event<down> /
                 []( UIModel &m, UIView &v ) {
                    --m.on_sec;
                    v.manual_program_draw_digit( UIView::next_highlight, 0, 1 );
                 },
         "setup_on_sec"_s
            + event<push> /
                 []( UIView &v ) {
                    v.manual_program_draw_digit( UIView::back_to_normal, 0, 1 );
                    v.manual_program_draw_digit( UIView::first_highlight, 1, 0 );
                 } = "setup_off_min"_s

         ,
         "setup_off_min"_s + event<up>[ off_minutes_top ],
         "setup_off_min"_s
            + event<up> /
                 []( UIModel &m, UIView &v ) {
                    ++m.off_min;
                    v.manual_program_draw_digit( UIView::next_highlight, 1, 0 );
                 },
         "setup_off_min"_s + event<down>[ off_minutes_bottom ],
         "setup_off_min"_s
            + event<down> /
                 []( UIModel &m, UIView &v ) {
                    --m.off_min;
                    v.manual_program_draw_digit( UIView::next_highlight, 1, 0 );
                 },
         "setup_off_min"_s
            + event<push> /
                 []( UIView &v ) {
                    v.manual_program_draw_digit( UIView::back_to_normal, 1, 0 );
                    v.manual_program_draw_digit( UIView::first_highlight, 1, 1 );
                 } = "setup_off_sec"_s

         ,
         "setup_off_sec"_s + event<up>[ off_seconds_top ],
         "setup_off_sec"_s
            + event<up> /
                 []( UIModel &m, UIView &v ) {
                    ++m.off_sec;
                    v.manual_program_draw_digit( UIView::next_highlight, 1, 1 );
                 },
         "setup_off_sec"_s + event<down>[ off_seconds_bottom ],
         "setup_off_sec"_s
            + event<down> /
                 []( UIModel &m, UIView &v ) {
                    --m.off_sec;
                    v.manual_program_draw_digit( UIView::next_highlight, 1, 1 );
                 },
         "setup_off_sec"_s + event<push> / call_next = X

      );
   }
};

struct program_selection
{
   auto operator()() const noexcept
   {
      using namespace sml;

      auto program_is_man = []( UIModel &m ) { return m.get_pgm() == 0; };
      auto prev_is_same   = []( UIModel &m ) { return m.get_pgm() == m.get_prev(); };
      auto next_is_same   = []( UIModel &m ) { return m.get_pgm() == m.get_next(); };

      return make_transition_table(
        * "program_selection_initial"_s + event<push> / [] (UIView &v) {
           v.draw_prog(true); } = "program_selected"_s
        , "program_selection_initial"_s + sml::on_entry<_> / [] (UIView &v) {
           v.draw_cursor(0); }
        , "program_selection_initial"_s + sml::on_exit<_> / [] (UIView &v) {
           v.erase_cursor(0); }
        , "program_selected"_s + event<up> [ prev_is_same ]
        , "program_selected"_s + event<up> / [] (UIModel &m, UIView &v) {
           m.set_pgm(m.get_prev());
           v.draw_prog(true); }
        , "program_selected"_s + event<down> [ next_is_same ]
        , "program_selected"_s + event<down> / [] (UIModel &m, UIView &v) {
           m.set_pgm(m.get_next());
           v.draw_prog(true); }
        , "program_selected"_s + event<push> [ program_is_man ] = state<program_setup>

        , state<program_setup> + sml::on_entry<_> / [] (UIView& v) {
           v.draw_program_setup_dialog(); }
        , state<program_setup> + sml::on_exit<_> / [] (UIModel &m, UIView& v) {
           m.store_manual_pgm();
           v.draw(); }
         , state<program_setup> +           event<next> / call_next = X
    );
   }
};

struct mode_manual
{
   auto operator()() const noexcept
   {
      using namespace sml;

      auto is_running = []( UIModel &m ) {
         return m.get_state() == UIModel::program_state_t::running;
      };
      auto is_paused = []( UIModel &m ) {
         return m.get_state() == UIModel::program_state_t::paused;
      };
      auto is_stopped = []( UIModel &m ) {
         return m.get_state() == UIModel::program_state_t::stopped;
      };

      return make_transition_table(
         *"select"_s + sml::on_entry<_> / call_next,
         "select"_s + event<next>[ is_running ] = state<walkman>,
         "select"_s + event<next>               = state<program_selection>

         ,
         state<walkman> + sml::on_exit<_> / []( UIView &v ) { v.draw_walkman(); },
         state<program_selection> + sml::on_exit<_> / []( UIView &v ) { v.erase_cursor( 1 ); },
         state<program_selection> + event<next> / []( UIModel &m ) { m.load_command(); } =
            state<walkman>

         ,
         "counter"_s + sml::on_entry<_> / []( UIView &v ) { v.draw_cursor( 2 ); },
         "counter"_s + sml::on_exit<_> / []( UIView &v ) { v.erase_cursor( 2 ); },
         "counter"_s + event<down> = "contact"_s, "counter"_s + event<up> = state<walkman>,
         "counter"_s
            + event<push> /
                 []( UIModel &m, UIView &v ) {
                    m.reset_counter();
                    v.draw_counter();
                 }

         ,
         "contact"_s + sml::on_entry<_> / []( UIView &v ) { v.draw_cursor( 3 ); },
         "contact"_s + sml::on_exit<_> / []( UIView &v ) { v.erase_cursor( 3 ); },
         "contact"_s + event<up>[ is_paused ] = "counter"_s,
         "contact"_s + event<up>              = state<walkman>,
         "contact"_s + event<push> / []( UIModel &m ) { m.flip_contact(); }

         ,
         state<program_selection> + event<down> = state<walkman>,
         state<program_selection> + event<push> / []( UIView &v ) { v.draw_prog( false ); } =
            state<walkman>,
         state<walkman> + event<down>[ is_paused ]  = "counter"_s,
         state<walkman> + event<down>[ is_stopped ] = "contact"_s,
         state<walkman> + event<up>[ is_stopped ]   = state<program_selection> );
   }
};

struct mode_usb
{};

struct sm_cyclo
{
   auto operator()() const noexcept
   {
      using namespace sml;

      return make_transition_table(
         *"splash"_s + event<splash_timeout> = state<mode_manual>,
         "splash"_s + sml::on_entry<_> / []( UIView &v ) { v.draw_splash(); }

         ,
         state<mode_manual> + sml::on_entry<_> / []( UIView &v ) { v.draw(); },
         state<mode_manual> + event<usb_on> / []( UIView &v ) { v.draw_usb(); } = state<mode_usb>,
         state<mode_usb> + event<usb_off> = state<mode_manual> );
   }
};

#endif  // ui_controller_h_included