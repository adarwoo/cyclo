#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

#include "logger.h"

#define LOG_F1   "f1"
#define LOG_MAIN "main"
#define LOG_OBJ  "obj"

#define INDENT "        "

using std::cout;

extern "C" void short_c_function();
extern "C" void a_c_function_with_a_very_long_name();

struct ItemFailed { };

class testClass_c
{
public:   testClass_c();
   ~testClass_c();
   void method() { LOG_THIS_HEADER( LOG_OBJ ); }
   void append_to_the_cycle_which_is_running_in_the_background_of_the_system()
   {
      LOG_THIS_HEADER( LOG_OBJ );
   }
};

testClass_c::testClass_c()
{
   LOG_THIS_HEADER( LOG_OBJ );
}

testClass_c::~testClass_c()
{
   LOG_THIS_HEADER( LOG_OBJ );
}

struct a_class_with_a_very_but_very_very_stupingly_long_name
{
   a_class_with_a_very_but_very_very_stupingly_long_name()
   {
      LOG_THIS_HEADER( LOG_OBJ );
   }

   ~a_class_with_a_very_but_very_very_stupingly_long_name()
   {
      LOG_THIS_HEADER( LOG_OBJ );
   }

   void a_method_of_the_class_with_a_very_but_very_very_stupingly_long_name()
   {
      LOG_THIS_HEADER( LOG_OBJ );
   }

   void method() { LOG_THIS_HEADER( LOG_OBJ ); }

};

void testCore()
{
   // Basic function test
   cout << "--> Checking mapping\n";
   cout << "  - Error level is " << LOG_LEVEL_ERROR << "\n";
   if ( LOG_LEVEL_ERROR != 0 ) { throw ItemFailed(); }
   cout << "  - Warn level is " << LOG_LEVEL_WARN << "\n";
   if ( LOG_LEVEL_WARN != 1) { throw ItemFailed(); }
   cout << "  - Mile level is " << LOG_LEVEL_MILE << "\n";
   if ( LOG_LEVEL_MILE != 2) { throw ItemFailed(); }
   cout << "  - Info level is " << LOG_LEVEL_INFO << "\n";
   if ( LOG_LEVEL_INFO != 3) { throw ItemFailed(); }
   cout << "  - Trace level is " << LOG_LEVEL_TRACE << "\n";
   if ( LOG_LEVEL_TRACE != 4) { throw ItemFailed(); }
   cout << "  - Log level is " << LOG_LEVEL_DEBUG << "\n";
   if ( LOG_LEVEL_DEBUG != 5) { throw ItemFailed(); }

   cout << "--> Checking basic messages (results vary according to command line)\n";
   LOG_ERROR( "dom.A", "ERROR level" );
   LOG_WARN(  "dom.A", "WARN level" );
   LOG_MILE(  "dom.A", "MILE level" );
   LOG_INFO(  "dom.A", "INFO level" );
   LOG_TRACE( "dom.A", "TRACE level" );
   LOG_DEBUG( "dom.A", "DEBUG level" );

   cout << "--> Checking C++ regulator and std::string output\n";
   std::string cppString="cpp string";
   LOG_INFO( "dom.A", cppString);
   LOG_INFO( "dom.A", cppString + "2" + "String");

   cout << "--> Checking domain level filtering\n";
   logDomainLevelPair_t *map;
   size_t i, found;
   size_t maxDomains = _log_get_limit( LOG_LIMIT_DOMAIN_LEVEL_FILTERS );
   size_t maxBuffer = _log_get_limit( LOG_LIMIT_DOMAIN_REPR ) + 1;

   /* Allocate the map and buffers */
   map = (logDomainLevelPair_t*)malloc( sizeof(logDomainLevelPair_t) * maxDomains );
   for (i=0; i<maxDomains; ++i)
   {
      map[i].domain = (char *)malloc( maxBuffer );
   }

   /* Query the log library */
   found = LOG_GETDOMAINLEVELS( map, maxBuffer, maxDomains );

   cout << "  - Found " << found << " settings\n";
   for (i=0; i<found; ++i)
   {
      cout << "   - " << map[i].domain << " : " << map[i].level << "\n";
   }

   /* Adding more domain using the API */
   cout << " --> Adding more domain filtering\n";
   cout << " + Setting domains X Y Z at trace, info and mile\n";
   LOG_SETDOMAINLEVEL("X", LOG_LEVEL_TRACE);
   LOG_SETDOMAINLEVEL("Y", LOG_LEVEL_INFO);
   LOG_SETDOMAINLEVEL("Z", LOG_LEVEL_MILE);

   /* Query the log library */
   found = LOG_GETDOMAINLEVELS( map, maxBuffer, maxDomains );

   cout << "  - Found " << found << " settings\n";
   for (i=0; i<found; ++i)
   {
      cout << "   - " << map[i].domain << " : " << map[i].level << "\n";
   }

   /* Override one */
   cout << " + Overriding X with error\n";
   LOG_SETDOMAINLEVEL("X", LOG_LEVEL_ERROR);

   found = LOG_GETDOMAINLEVELS( map, maxBuffer, maxDomains );

   cout << "  - Found " << found << " settings\n";
   for (i=0; i<found; ++i)
   {
      cout << "   - " << map[i].domain << " : " << map[i].level << "\n";
   }

   /** Clear one */
   cout << " + Removing X filtering\n";
   LOG_CLEARDOMAINLEVEL("X");

   found = LOG_GETDOMAINLEVELS( map, maxBuffer, maxDomains );

   cout << "  - Found " << found << " settings\n";
   for (i=0; i<found; ++i)
   {
      cout << "   - " << map[i].domain << " : " << map[i].level << "\n";
   }

   /** Clear all ! */
   cout << " + Removing all\n";
   LOG_CLEARDOMAINLEVEL(0);
   found = LOG_GETDOMAINLEVELS( map, maxBuffer, maxDomains );

   cout << "  - Found " << found << " settings\n";
   for (i=0; i<found; ++i)
   {
      cout << "   - " << map[i].domain << " : " << map[i].level << "\n";
   }

   // Clean-up
   for (i=0; i<maxDomains; ++i)
   {
      free(map[i].domain);
   }
   free(map);

   cout << "--> Checking level settings\n";
   int level = LOG_GETLEVEL();
   cout << "  -Level is : " << level << "\n";
   cout << "  +Setting level to ERROR\n";
   LOG_SETLEVEL( LOG_LEVEL_ERROR );
   if ( LOG_GETLEVEL() != LOG_LEVEL_ERROR ) { throw ItemFailed(); }
   cout << "  +Setting level to WARN\n";
   LOG_SETLEVEL( LOG_LEVEL_WARN );
   if ( LOG_GETLEVEL() != LOG_LEVEL_WARN ) { throw ItemFailed(); }
   cout << "  +Setting level to MILE\n";
   LOG_SETLEVEL( LOG_LEVEL_MILE );
   if ( LOG_GETLEVEL() != LOG_LEVEL_MILE ) { throw ItemFailed(); }
   cout << "  +Setting level to INFO\n";
   LOG_SETLEVEL( LOG_LEVEL_INFO );
   if ( LOG_GETLEVEL() != LOG_LEVEL_INFO ) { throw ItemFailed(); }
   cout << "  +Setting level to TRACE\n";
   LOG_SETLEVEL( LOG_LEVEL_TRACE );
   if ( LOG_GETLEVEL() != LOG_LEVEL_TRACE ) { throw ItemFailed(); }
   cout << "  +Setting level to DEBUG\n";
   LOG_SETLEVEL( LOG_LEVEL_DEBUG );
   if ( LOG_GETLEVEL() != LOG_LEVEL_DEBUG ) { throw ItemFailed(); }
   cout << "  +Restoring level\n";
   LOG_SETLEVEL( level );
   if ( LOG_GETLEVEL() != level ) { throw ItemFailed(); }

   cout << "--> Checking masks (domain filtering) setting\n";
   size_t max = LOG_GETLIMIT( LOG_LIMIT_MASKS );
   char *oldMasks = (char *)malloc(max);
   char *oldNotMasks = (char *)malloc(max);

   LOG_GETMASKS(oldMasks, max, oldNotMasks, max );
   cout << "  - Shown masks are " << oldMasks << "\n";
   cout << "  - Hidden masks are " << oldNotMasks << "\n";

   cout << "  + Setting mask to A:B:C\n";
   LOG_MASK( "A:B:C" );
   char *newMasks = (char *)malloc(max);
   LOG_GETMASKS(newMasks, max, 0, 0 );
   if ( strcmp("A:B:C", newMasks) != 0 ) { throw ItemFailed(); }

   cout << "  + Setting not mask to D:E:F\n";
   LOG_NOTMASK( "D:E:F" );
   char *newNotMasks = (char *)malloc(max);
   LOG_GETMASKS(0, 0, newNotMasks, max );
   if ( strcmp( "D:E:F", newNotMasks) != 0 ) { throw ItemFailed(); }

   cout << "  + Restoring mask filtering\n";
   LOG_MASK( oldMasks );
   LOG_NOTMASK( oldNotMasks );
   LOG_GETMASKS(newMasks, max, newNotMasks, max );
   if ( strcmp( oldMasks, newMasks) != 0 || strcmp( oldNotMasks, newNotMasks) != 0 )
   {
      throw ItemFailed();
   }
   free( oldMasks ); free( oldNotMasks );
   free( newMasks ); free( newNotMasks );
}

void function1()
{
   LOG_HEADER( LOG_F1 );

   printf("Adding a DEBUG comment\n");
   LOG_DEBUG( LOG_F1, "Entering" );

   printf("Adding a TRACE comment\n");
   LOG_WARN( LOG_F1, "a warning message" );

   printf("Adding a DEBUG comment\n");
   LOG_DEBUG( LOG_F1, "Leaving" );
}

void test_streams()
{
   printf("Adding a DEBUG stream comment\n");
   LOG_STREAM( LOG_MAIN, LOG_LEVEL_DEBUG, "Stream test " << LOG_LEVEL_DEBUG << " DEBUG" );

   printf("Adding a TRACE stream comment\n");
   LOG_STREAM( LOG_MAIN, LOG_LEVEL_TRACE, "Stream test " << LOG_LEVEL_TRACE << " TRACE" );
}

void test_indent_f1()
{
   LOG_DEBUG( LOG_MAIN, "Different function");
}

void test_indent_main()
{
   LOG_DEBUG( LOG_MAIN, "New thread, no indent");
   test_indent_f1();
   LOG_DEBUG( LOG_MAIN, "Back in context");
   LOG_DEBUG( LOG_MAIN, "Same function, different line");
   LOG_DEBUG( LOG_F1, "Change domain");
   short_c_function();
}

void domainsLog()
{
   LOG_DEBUG( "root", "root" );
   LOG_DEBUG( "root.node1", "root.node1" );
   LOG_DEBUG( "root.node1.leaf1", "root.node1.leaf1" );
   LOG_DEBUG( "root.node1.leaf2", "root.node1.leaf2" );
   LOG_DEBUG( "root.node2.leaf1", "root.node2.leaf1" );
   LOG_DEBUG( "root.node2.leaf2", "root.node2.leaf2" );
   if ( LOG_CHECK( "root.node1.leaf1", LOG_LEVEL_INFO ) )
   {
      LOG_ERROR( "root.node1.leaf1", "root.node1.leaf1" );
   }
   if ( LOG_CHECK( "root.node2.leaf1", LOG_LEVEL_DEBUG ) )
   {
      LOG_ERROR( "root.node2.leaf1", "root.node2.leaf1" );
   }
}

int main( int argc, char **argv )
{
   testCore();
   //
   //   Checking formating, indent etc..
   //
   printf( INDENT "Testing indent\n");
   test_indent_main();

   LOG_DEBUG( LOG_MAIN, "Checking truncation");
   a_c_function_with_a_very_long_name();

   printf( INDENT "Adding a ERROR comment\n");
   LOG_ERROR( LOG_MAIN, "Starting the test for log" );

   printf("Adding a MILE comment\n");
   LOG_MILE( LOG_MAIN, "Starting the test for log" );

   int nLogLevel = LOG_GETLEVEL();
   printf("Current debug level is  %d\n", nLogLevel );

   printf("Adding a TRACE comment\n");
   LOG_TRACE( LOG_MAIN, "Tracing some text" );

   printf("Calling function1\n");
   function1();

   printf("Creating an object\n");
   testClass_c myObj = testClass_c();
   myObj.method();
   myObj.append_to_the_cycle_which_is_running_in_the_background_of_the_system();

   printf("Creating an object with a long name (should be truncated)\n");
   a_class_with_a_very_but_very_very_stupingly_long_name myObj2;
   myObj2.a_method_of_the_class_with_a_very_but_very_very_stupingly_long_name();
   myObj2.method();

   printf("Adding a TRACE comment\n");
   LOG_TRACE( LOG_MAIN, "Between calls" );

   function1();

   nLogLevel = LOG_GETLEVEL();
   printf("Current debug level is  %d\n", nLogLevel );

   printf("Adding a MILE comment\n");
   LOG_MILE( LOG_MAIN, "End of the test for log" );

   printf("Activating domains F1 and MAIN\n");
   LOG_MASK( "main,f1" );

   printf("Debug 3 domains\n");
   LOG_MILE( LOG_MAIN, "domain is main" );
   LOG_MILE( LOG_F1, "domain is f1" );
   LOG_MILE( LOG_OBJ, "domain is obj" );

   printf("Activating too many domains...\n");
   LOG_MASK(
      "main,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,"
      "f11,f12,f13,f14,f15,f16,f17,fveryveryveryveryveryverylong"
   );

   printf("Debug 3 domains\n");
   LOG_MILE( LOG_MAIN, "domain is main" );
   LOG_MILE( LOG_F1, "domain is f1" );
   LOG_MILE( LOG_OBJ, "domain is obj" );

   printf("Setting all domains...\n");
   LOG_MASK( "" );

   printf("Debug 3 domains\n");
   LOG_MILE( LOG_MAIN, "domain is main" );
   LOG_MILE( LOG_F1, "domain is f1" );
   LOG_MILE( LOG_OBJ, "domain is obj" );

   printf("Testing streaming\n");
   test_streams();

   printf("Testing wildcard\n");

   printf("   -> Simple domain 'root'\n");
   LOG_CLEARDOMAINLEVEL(0);
   LOG_SETDOMAINLEVEL("root", LOG_LEVEL_DEBUG);
   domainsLog();

   printf("   -> Exact domain 'root.node1.leaf1'\n");
   LOG_CLEARDOMAINLEVEL(0);
   LOG_SETDOMAINLEVEL("root.node1.leaf1", LOG_LEVEL_DEBUG);
   domainsLog();

   printf("   -> Simple wild '*'\n");
   LOG_CLEARDOMAINLEVEL(0);
   LOG_SETDOMAINLEVEL("*", LOG_LEVEL_DEBUG);
   domainsLog();

   printf("   -> Simple wild 'root.*'\n");
   LOG_CLEARDOMAINLEVEL(0);
   LOG_SETDOMAINLEVEL("root.*", LOG_LEVEL_DEBUG);
   domainsLog();

   printf("   -> Simple domain 'root*'\n");
   LOG_CLEARDOMAINLEVEL(0);
   LOG_SETDOMAINLEVEL("root*", LOG_LEVEL_DEBUG);
   domainsLog();

   printf("   -> Use of ? 'root.node1.*'\n");
   LOG_CLEARDOMAINLEVEL(0);
   LOG_SETDOMAINLEVEL("root.node1.*", LOG_LEVEL_DEBUG);
   domainsLog();

   printf("   -> Use of ? 'root.node?.leaf*'\n");
   LOG_CLEARDOMAINLEVEL(0);
   LOG_SETDOMAINLEVEL("root.node?.leaf*", LOG_LEVEL_DEBUG);
   domainsLog();

   printf("   -> Mixing levels. root.*=root.node?.*=trace and root.node1.*=debug\n");
   LOG_CLEARDOMAINLEVEL(0);
   LOG_SETDOMAINLEVEL("root.*", LOG_LEVEL_TRACE);
   LOG_SETDOMAINLEVEL("root.node?.*", LOG_LEVEL_TRACE);
   LOG_SETDOMAINLEVEL("root.node1.*", LOG_LEVEL_DEBUG);
   domainsLog();

   // Get input param for the domain redirection test
   if (argc < 2)
   {
      printf("Missing input param for domain redirection test\n");
   }
   else
   {
      printf("   -> Mixing domain redirection. root.node1.*=%s\n", argv[1]);
      LOG_CLEARDOMAINLEVEL(0);
      LOG_SETDOMAINLEVEL("root.*", LOG_LEVEL_DEBUG);
      FILE *pf = fopen( argv[1], "w" );
      if (pf != NULL)
      {
         LOG_SETDOMAINREDIRECTION("root.node1.*", pf);
      }
      domainsLog();
      fclose(pf);
   }

   return 0;
}
