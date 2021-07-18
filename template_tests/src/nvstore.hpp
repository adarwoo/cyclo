/*
 * Store the shell history and the programs
 * The EEProm is divided into a entry allocation table made of fixed sized items, pointing
 *  to variable size items
 *
 * Created: 17/07/2021 18:22:20
 *  Author: micro
 */ 

#include "asf.h"
#include "conf_cyclo.hpp"

#include "etl/bitset.h"
#include "etl/string_view.h"

/** 
 * Storage in NV of the various persitable items
 */
class NVStore
{
public:
   using Pgms = etl::bitset<10>;
   constexpr static size_t STORAGE_MAX_LENGTH = 60;
   
   template<typename T>
   T *pgm_mapped_at(const size_t pgmIndex)
   {
      void *address = reinterpret_cast<void*>(pgmIndex + MAPPED_EEPROM_START);
      
      return static_cast<T*>(address);
   }

   eeprom_addr_t pgm_at(const size_t index)
   {
      return EEPROM_PAGE_SIZE * index * (STORAGE_MAX_LENGTH / EEPROM_PAGE_SIZE);
   }
   
   static auto& instance(){
      static NVStore _this;
      return _this;
   }
      
private:
   Pgms occupancy_map;
   uint8_t default_pgm;

   NVStore(const NVStore&) = delete;
   NVStore& operator=(const NVStore &) = delete;
   NVStore(NVStore &&) = delete;
   NVStore & operator=(NVStore &&) = delete;
   
   NVStore() : default_pgm{0}
   {
	   nvm_wait_until_ready();
	   eeprom_enable_mapping();
      
      // Read the pages, looking for entries
      for ( uint8_t i=0; i<10; ++i)
      {
         // Address of the block
         uint8_t *marker_loc = pgm_mapped_at<uint8_t>(i);
         
         if ( *marker_loc != 0xFF )
         {
            // Compute the crc for the slot. Add one for the marker
            uint16_t crc = crc_io_checksum(marker_loc, STORAGE_MAX_LENGTH + 1, CRC_16BIT);
           
            uint16_t actual_crc = *((uint16_t*)(marker_loc+STORAGE_MAX_LENGTH+1));
            
            if ( crc == actual_crc )
            {
               occupancy_map.set(i);
               
               // Is it the default?
               if ( *marker_loc == '*' )
               {
                  default_pgm = i;
               }
            }
         }
      }   
   }
   
   /** Return the number of items stored */
   etl::string_view get_pgm(uint8_t pgm)
   {
	   nvm_wait_until_ready();
	   eeprom_enable_mapping();
      char *mapped_addr = pgm_mapped_at<char>(pgm);
      return etl::string_view(mapped_addr);
   }
   

   /** Delete an item of the given type at the given position */
   void delete_pgm(uint8_t pgm)
   {
      
   }
   
   void write_pgm_at(uint8_t, etl::string_view string)
   {
      
   }
};

