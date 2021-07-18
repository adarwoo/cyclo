#ifndef pgm_man_included
#define  pgm_man_included
/*
 * pgm_man.cpp
 *
 * Created: 17/07/2021 18:28:51
 *  Author: micro
 */ 
#include "cyclo.hpp"
#include "nvstore.hpp"


class PgmStore
{
   NVStore &store;
   cyclo::Command pgms[10];
   
public:
   PgmStore(NVStore &store) : store {store}
   {
      // Read from the nvstore the saved programs
      store.get
   }
      
   cyclo::pgm_sel get_pgm() const 
   
};



#endif // ndef pgm_man_included