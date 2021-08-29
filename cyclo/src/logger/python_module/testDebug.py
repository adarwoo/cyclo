#!/usr/bin/env python
# -*- coding: latin-1 -*-

import LOG
import gc


def funcA():
    # line
    # line
    # line
    # line
    # line
    # line
    # line
    # line
    # line
    # line
    # line
    # line
    # line
    LOG.ERROR("other", "This should be in line 18")


def funcB():
    poem_by_Rabelais = """
         Heureux qui, comme Ulysse, a fait un beau voyage,
         Ou comme cestuy-l� qui conquit la toison,
         Et puis est retourn�, plein d'usage et raison,
         Vivre entre ses parents le reste de son �ge !

         Quand reverrai-je, h�las, de mon petit village
         Fumer la chemin�e, et en quelle saison
         Reverrai-je le clos de ma pauvre maison,
         Qui m'est une province, et beaucoup davantage ?

         Plus me pla�t le s�jour qu'ont b�ti mes a�eux,
         Que des palais Romains le front audacieux,
         Plus que le marbre dur me pla�t l'ardoise fine :

         Plus mon Loir gaulois, que le Tibre latin,
         Plus mon petit Lir�, que le mont Palatin,
         Et plus que l'air marin la doulceur angevine.
         Heureux qui, comme Ulysse, a fait un beau voyage,
         Ou comme cestuy-l� qui conquit la toison,
         Et puis est retourn�, plein d'usage et raison,
         Vivre entre ses parents le reste de son �ge !

         Quand reverrai-je, h�las, de mon petit village
         Fumer la chemin�e, et en quelle saison
         Reverrai-je le clos de ma pauvre maison,
         Qui m'est une province, et beaucoup davantage ?

         Plus me pla�t le s�jour qu'ont b�ti mes a�eux,
         Que des palais Romains le front audacieux,
         Plus que le marbre dur me pla�t l'ardoise fine :

         Plus mon Loir gaulois, que le Tibre latin,
         Plus mon petit Lir�, que le mont Palatin,
         Et plus que l'air marin la doulceur angevine.
         Heureux qui, comme Ulysse, a fait un beau voyage,
         Ou comme cestuy-l� qui conquit la toison,
         Et puis est retourn�, plein d'usage et raison,
         Vivre entre ses parents le reste de son �ge !

         Quand reverrai-je, h�las, de mon petit village
         Fumer la chemin�e, et en quelle saison
         Reverrai-je le clos de ma pauvre maison,
         Qui m'est une province, et beaucoup davantage ?

         Plus me pla�t le s�jour qu'ont b�ti mes a�eux,
         Que des palais Romains le front audacieux,
         Plus que le marbre dur me pla�t l'ardoise fine :

         Plus mon Loir gaulois, que le Tibre latin,
         Plus mon petit Lir�, que le mont Palatin,
         Et plus que l'air marin la doulceur angevine.
   """
    LOG.DEBUG("TEST", poem_by_Rabelais)


def A_Function_with_a_stupidely_long_name():
    LOG.MILE("very_long_domain_name", "From the function",
             A_Function_with_a_stupidely_long_name)


def A_Function_with_a_even_more_stupidely_long_name_which_definitly_ought_to_be_truncated():
    LOG.MILE("very_long_domain_name", "From the function",
             A_Function_with_a_stupidely_long_name)


class A:
    def __init__(self):
        LOG.DEBUG("tst", "Enter")

    def tduc(self):
        LOG.DEBUG("tst", "In tduc")

    def A_function_with_a_long_name(self):
        LOG.TRACE("tst", "From A_function_with_a_long_name")


class A_Class_with_a_stupidely_long_name:
    def A_function_with_a_long_name(self):
        LOG.TRACE(
            "tst", "From A_function_with_a_long_name of the class A_Class_with_a_stupidely_long_name")


funcA()
funcB()

LOG.ERROR("tst", "ERROR level", "More", "More")
LOG.WARN("tst", "WARN level - int - float - list - tuple", 45, 32.45,
         ["a", 324, 43.4, (34, "fd")], ([4, 5], "sad", None, True))
LOG.MILE("tst", "MILE level")
LOG.TRACE("tst", "TRACE level")
LOG.DEBUG("tst", "LOG level")

A_Function_with_a_stupidely_long_name()

A_Function_with_a_even_more_stupidely_long_name_which_definitly_ought_to_be_truncated()

a = A()
a.tduc()
a.A_function_with_a_long_name()
b = A()
funcA()
c = A_Class_with_a_stupidely_long_name()
c.A_function_with_a_long_name()
