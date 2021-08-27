/**
 * @file
 * @{
 * Test the embedded version of the logger
 * This version uses a more restrictive setting
 */
#include "logger.h"

extern void short_c_function();
extern void a_c_function_with_a_very_long_name();

int main()
{
   LOG_INIT("debug");
   LOG_SETLEVEL(LOG_LEVEL_DEBUG);

   short_c_function();
   a_c_function_with_a_very_long_name();

   // Testing unformatted print function
   LOG_PRINT("Hello world! in %d", 2020);
   LOG_PRINT(
         "Heureux qui, comme Ulysse, a fait un beau voyage,\n"
         "Ou comme cestuy-la qui conquit la toison,\n"
         "Et puis est retourne, plein d'usage et raison,\n"
         "Vivre entre ses parents le reste de son age !\n"
         "\n"
         "Quand reverrai-je, helas, de mon petit village\n"
         "Fumer la cheminee, et en quelle saison\n"
         "Reverrai-je le clos de ma pauvre maison,\n"
         "Qui m'est une province, et beaucoup davantage ?\n"
         "\n"
         "Plus me plait le sejour qu'ont bati mes aieux,\n"
         "Que des palais Romains le front audacieux,\n"
         "Plus que le marbre dur me plait l'ardoise fine :\n"
         "\n"
         "Plus mon Loir gaulois, que le Tibre latin,\n"
         "Plus mon petit Lire, que le mont Palatin,\n"
         "Et plus que l'air marin la doulceur angevine.\n"
         "Heureux qui, comme Ulysse, a fait un beau voyage,\n"
         "Ou comme cestuy-la qui conquit la toison,\n"
         "Et puis est retourne, plein d'usage et raison,\n"
         "Vivre entre ses parents le reste de son age !\n"
         "\n"
         "Quand reverrai-je, helas, de mon petit village\n"
         "Fumer la cheminee, et en quelle saison\n"
         "Reverrai-je le clos de ma pauvre maison,\n"
         "Qui m'est une province, et beaucoup davantage ?\n"
         "\n"
         "Plus me plait le sejour qu'ont bati mes aieux,\n"
         "Que des palais Romains le front audacieux,\n"
         "Plus que le marbre dur me plait l'ardoise fine :\n"
         "\n"
         "Plus mon Loir gaulois, que le Tibre latin,\n"
         "Plus mon petit Lire, que le mont Palatin,\n"
         "Et plus que l'air marin la doulceur angevine.\n"
         "Heureux qui, comme Ulysse, a fait un beau voyage,\n"
         "Ou comme cestuy-la qui conquit la toison,\n"
         "Et puis est retourne, plein d'usage et raison,\n"
         "Vivre entre ses parents le reste de son age !\n"
         "\n"
         "Quand reverrai-je, helas, de mon petit village\n"
         "Fumer la cheminee, et en quelle saison\n"
         "Reverrai-je le clos de ma pauvre maison,\n"
         "Qui m'est une province, et beaucoup davantage ?\n"
         "\n"
         "Plus me plait le sejour qu'ont bati mes aieux,\n"
         "Que des palais Romains le front audacieux,\n"
         "Plus que le marbre dur me plait l'ardoise fine :\n"
         "\n"
         "Plus mon Loir gaulois, que le Tibre latin,\n"
         "Plus mon petit Lire, que le mont Palatin,\n"
         "Et plus que l'air marin la doulceur angevine.\n"
   );

   return 0;
}
