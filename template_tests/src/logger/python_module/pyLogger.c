/*-
 *****************************************************************************
 * log module for Python. Uses the C module for common debugging strategy
 *  between C/C++ and Python
 *
 * @author gpa
 *****************************************************************************
 */


/* ---------------------------------------------------------------------------
 * Includes
 * -------------------------------------------------------------------------*/
#include <Python.h>
#include <frameobject.h>
#include "logger.h"


/* ---------------------------------------------------------------------------
 * Local defines
 * -------------------------------------------------------------------------*/
#define MAX_FIELD_LENGTH   22
#define ELIPSIS_LENGTH      3
#define DOUBLE_COLON_LENGTH 2
#define ZERO_CHAR_LENGTH    1

/** Max size for temp string buffers */
#define STR_SIZE_MAX        2000


/* ---------------------------------------------------------------------------
 * Python 2/3 compatibility macros
 * -------------------------------------------------------------------------*/
#if PY_MAJOR_VERSION >= 3
#  define PyString_FromString PyUnicode_FromString
#  define PyString_AS_STRING PyUnicode_AsUTF8
#  define PyInt_FromLong PyLong_FromLong
#  define PyString_Check PyBytes_Check
#  define MOD_INIT(name) PyMODINIT_FUNC PyInit_##name(void)
#  define MOD_SUCCESS_VAL(val) val
#  define MOD_ERROR_VAL NULL
#else
#  define MOD_SUCCESS_VAL(val)
#  define MOD_ERROR_VAL
#  define MOD_INIT(name) void init##name(void)
#endif


/* ---------------------------------------------------------------------------
 * Module globals
 * -------------------------------------------------------------------------*/
static PyObject *log_error = NULL;

/** Two buffer for the method name so debug detects a change */
static char s_class_method_name[2][MAX_FIELD_LENGTH*2 + DOUBLE_COLON_LENGTH + ZERO_CHAR_LENGTH];

/** Which function buffer do we use */
static int s_class_method_switch = 0;


/**
 *****************************************************************************
 * Helper to truncated a string and show the truncation
 * Returns a pointer to the string and sets the truncation buffer pointer if
 *  the string had to be truncated
 *
 * @param   p  String to truncate
 * @param   maxLength   Up to
 * @param   pTruncationMarkBuffer   A string buffer pointer to set to a
 *           truncation string if truncated. Left unchanged if no truncation
 * @return  A pointer to the truncated string, that points to the original
             string
 *****************************************************************************
 */
const char *get_truncated_string(
   const char *p,
   int maxLength,
   const char * * pTruncationMarkBuffer )
{
   static const char * const wasTruncated="...";
   int length = strlen(p) - maxLength - 1; /* \0 */
   
   if ( length > 0 )
   {
      *pTruncationMarkBuffer = wasTruncated;
      return &p[length+sizeof(wasTruncated)];
   }
   
   return p;
}


/**
 *****************************************************************************
 * Get hold of the frame state for a call
 * To force the debug API to detect that the function name has changed, we
 *  must change to actual storage.
 * So everytime we get the frame info, the alternate the buffer. If the
 *  method name has actually changed, the switch is reversed.
 *****************************************************************************
 */
static void get_frame_info()
{
   PyFrameObject *f;
   PyObject *map, **fast;
   const char *part;
   int nmap, j;

   const char *pClassTrunc, *pClass, *pComma, *pFunctionTrunc, *pFunction;

   /* Reset function name segments */
   pClassTrunc = pClass = pComma = pFunctionTrunc = pFunction = NULL;

   /* Get hold of the thread frame context */
   f = PyThreadState_GET()->frame;

   /* Switch function buffer */
   s_class_method_switch = 1 - s_class_method_switch;
   char* class_method_name = s_class_method_name[s_class_method_switch];

   /* Force default */
   strcpy(class_method_name, "?");
   _log_filename = _log_function  = class_method_name;
   _log_line = 0;

   if (f != NULL)
   {
      /* Extract immediate info from the structure */
      _log_filename = PyString_AS_STRING(f->f_code->co_filename);
      _log_line = (f->f_trace) ? f->f_lineno : PyCode_Addr2Line(f->f_code, f->f_lasti);

      class_method_name[0] = '\0';
      _log_function = class_method_name;

      /*
       * Sadly the actual f_local field is unset here.
       * This information is available from a fast map which normally would
       *  get converted to a dictionary.
       * To avoid the extra processing for the conversion we exploit the fast
       *  map directly.
       */
      map = f->f_code->co_varnames;

      if (PyTuple_Check(map))
      {
         fast = f->f_localsplus;
         nmap = f->f_code->co_nlocals;

         for (j = nmap; --j >= 0; )
         {
            PyObject *key = PyTuple_GET_ITEM(map, j);
            PyObject *value = fast[j];

            /* If one of the local variable is 'self', good chance are this is a class */
            if ( PyString_Check(key) && strcmp(PyString_AS_STRING(key), "self")==0)
            {
               /* Get hold the __class__ attribute of the local variable */
               PyObject *class_attr = PyObject_GetAttrString(value, "__class__");

               if ( class_attr )
               {
                  PyObject *name_attr = PyObject_GetAttrString(class_attr, "__name__");

                  if ( PyString_Check(name_attr) )
                  {
                     pClass = get_truncated_string(
                        PyString_AS_STRING(name_attr), MAX_FIELD_LENGTH, &pClassTrunc );

                     pComma = "::";
                  }
                  Py_CLEAR(name_attr);
               }
               Py_CLEAR(class_attr);
            }
         }
      }

      /* Add the method name */
      pFunction = get_truncated_string(
         PyString_AS_STRING(f->f_code->co_name),
         pClass ? (2 * MAX_FIELD_LENGTH) - strlen(pClass) : 2 * MAX_FIELD_LENGTH,
         &pFunctionTrunc );

      /* Create final name */
      for ( j=0; j<=4; ++j )
      {
         switch (j)
         {
            case 0:  part=pClassTrunc;    break;
            case 1:  part=pClass;         break;
            case 2:  part=pComma;         break;
            case 3:  part=pFunctionTrunc; break;
            default: part=pFunction;      break;
         }

         while ( part && *part )
         {
            *class_method_name++ = *part++;
         }

         *class_method_name = '\0';
      }
   }

   /* Has the function name not changed ? */
   if ( strcmp( s_class_method_name[0], s_class_method_name[1] ) == 0 )
   {
      /* Switch function back to previous buffer */
      s_class_method_switch = 1 - s_class_method_switch;
      _log_function = s_class_method_name[s_class_method_switch];
   }
}


/**
 *****************************************************************************
 * Check args and compact them to send to the debug trace function.
 * The function should never cause an error. Instead a log error would
 *  be sent if the call was malformed.
 *
 * @param Usual for a Python function
 *****************************************************************************
 */
static PyObject *py_trace(int level, PyObject *self, PyObject *args)
{
   int i;
   const char *domain = "all";
   size_t sizemax = STR_SIZE_MAX;
   static const char elipsis[] = "...";
   static char theBigString[STR_SIZE_MAX + sizeof(elipsis) + 1];

   theBigString[0]='\0';

   /* Get hold of the frame info and set line, function, etc.. */
   get_frame_info();

   /* Get hold of the big tuple */
   if ( PyTuple_Check( args ) )
   {
      int n = PyTuple_GET_SIZE(args);
      PyObject *it = 0;

      if ( n )
      {
         it = PyTuple_GetItem(args, 0);

         if ( PyString_Check(it) )
         {
            domain = PyString_AS_STRING(it);
         }
      }

      for ( i=1; i<n; i++)
      {
         const char *item;
         int len;
         PyObject *pyObj;

         /* Read al tuple args */
         it = PyTuple_GetItem(args, i);
         pyObj = PyObject_Str(it);
         item = PyString_AS_STRING(pyObj);
         len = strlen(item);

         strncat( theBigString, item, sizemax );
         strcat( theBigString, " " );

         sizemax -= (len + 1);

         if ( sizemax < 3 )
         {
            strcat( theBigString, elipsis );
            break;
         }

         Py_CLEAR( pyObj );
      }
   }

   _log_type = level;
   _log_trace( domain, "%s", theBigString );

   Py_RETURN_NONE;
}


/**
 *****************************************************************************
 * Generate a log level trace
 * The test for the level is done immediatly to shorten execution time
 *****************************************************************************
 */
static PyObject *pylog_error(PyObject *self, PyObject *args)
{
   return py_trace(LOG_LEVEL_ERROR, self, args);
}

static PyObject *pylog_warn(PyObject *self, PyObject *args)
{
   if ( ! _LOG_DOMAIN_LEVEL_LOOKUP_IS_EMPTY ||_log_level >= LOG_LEVEL_WARN )
   {
      return py_trace(LOG_LEVEL_WARN, self, args);
   }

   Py_RETURN_NONE;
}

static PyObject *pylog_mile(PyObject *self, PyObject *args)
{
   if ( ! _LOG_DOMAIN_LEVEL_LOOKUP_IS_EMPTY || _log_level >= LOG_LEVEL_MILE )
   {
      return py_trace(LOG_LEVEL_MILE, self, args);
   }

   Py_RETURN_NONE;
}

static PyObject *pylog_info(PyObject *self, PyObject *args)
{
   if ( ! _LOG_DOMAIN_LEVEL_LOOKUP_IS_EMPTY || _log_level >= LOG_LEVEL_INFO )
   {
      return py_trace(LOG_LEVEL_INFO, self, args);
   }

   Py_RETURN_NONE;
}

static PyObject *pylog_trace(PyObject *self, PyObject *args)
{
   if ( ! _LOG_DOMAIN_LEVEL_LOOKUP_IS_EMPTY ||_log_level >= LOG_LEVEL_TRACE )
   {
      return py_trace(LOG_LEVEL_TRACE, self, args);
   }

   Py_RETURN_NONE;
}

static PyObject *pylog_debug(PyObject *self, PyObject *args)
{
   if ( ! _LOG_DOMAIN_LEVEL_LOOKUP_IS_EMPTY ||_log_level >= LOG_LEVEL_DEBUG )
   {
      return py_trace(LOG_LEVEL_DEBUG, self, args);
   }
   
   Py_RETURN_NONE;
}


static PyObject *pylog_getlevel(PyObject *self, PyObject *args)
{
   return PyInt_FromLong( _log_level );
}

static PyObject *pylog_setlevel(PyObject *self, PyObject *args)
{
   int level;
   
   if (! PyArg_ParseTuple(args, "i", &level))
   {
      return NULL;
   }

   if ( level <= LOG_LEVEL_DEBUG )
   {
      _log_level = level;
   }
   else
   {
      return NULL;
   }

   Py_RETURN_NONE;
}

static PyObject *pylog_mask(PyObject *self, PyObject *args)
{
   char *pMask;
   
   if (! PyArg_ParseTuple(args, "z:", &pMask))
   {
      return NULL;
   }

   _log_set_mask( pMask );

   Py_RETURN_NONE;
}

static PyObject *pylog_not_mask(PyObject *self, PyObject *args)
{
   char *pNotMask;
   
   if (! PyArg_ParseTuple(args, "z:", &pNotMask))
   {
      return NULL;
   }

   _log_set_not_mask( pNotMask );

   Py_RETURN_NONE;
}

/** Returns a tuple of strings with the mask/notMask */
static PyObject *pylog_get_masks(PyObject *self, PyObject *args)
{
   PyObject *pyMask, *pyNotMask, *pyTuple;
   size_t max = _log_get_limit( LOG_LIMIT_MASKS );
   char *masks = (char *)malloc(max);
   char *notMasks = (char *)malloc(max);

   _log_get_masks( masks, max, notMasks, max );

   /* Create a tuple of strings */
   pyMask = PyString_FromString(masks);
   pyNotMask = PyString_FromString(notMasks);
   free( masks );
   free( notMasks );

   pyTuple = PyTuple_New( 2 );
   PyTuple_SetItem( pyTuple, 0, pyMask );
   PyTuple_SetItem( pyTuple, 1, pyNotMask );

   return pyTuple;
}

static PyObject *pylog_set_domain_level(PyObject *self, PyObject *args)
{
   int level = LOG_LEVEL_ERROR;
   char *mask;

   if (! PyArg_ParseTuple(args, "z|i:", &mask, &level))
   {
      return NULL;
   }

   if ( level <= LOG_LEVEL_DEBUG )
   {
      _log_set_domain_level( mask, level );
   }

   Py_RETURN_NONE;
}

static PyObject *pylog_get_domain_levels(PyObject *self, PyObject *args)
{
   logDomainLevelPair_t *map;
   size_t i, found;
   size_t maxDomains = _log_get_limit( LOG_LIMIT_DOMAIN_LEVEL_FILTERS );
   size_t maxBuffer = _log_get_limit( LOG_LIMIT_DOMAIN_REPR ) + 1;
   PyObject *pyList;

   /* Allocate the map and buffers */
   map = malloc( sizeof(logDomainLevelPair_t) * maxDomains );
   
   for (i=0; i<maxDomains; ++i)
   {
      map[i].domain = (char *)malloc( maxBuffer );
   }

   /* Query the log library */
   found = _log_get_domain_levels( map, maxBuffer, maxDomains);

   /* New Python list object */
   pyList = PyList_New( found );

   /* Return a list of tuples [ ( dom1, level ) ( dom2, level2 ) ] */
   for ( i=0; i<found; ++i)
   {
      PyObject *pyTuple = PyTuple_New( 2 );
      PyTuple_SetItem( pyTuple, 0, PyString_FromString(map[i].domain) );
      PyTuple_SetItem( pyTuple, 1, PyInt_FromLong(map[i].level) );

      PyList_SetItem( pyList, i, pyTuple );
   }

   /* Clean-up temporally map */
   for (i=0; i<maxDomains; ++i)
   {
      free( map[i].domain );
   }

   free( map );

   return pyList;
}

/* Module functions */
static PyMethodDef methods[] =
{
   {
      "ERROR", pylog_error, METH_VARARGS,
      "Output an 'error' level entry"
   },
   {
      "WARN", pylog_warn, METH_VARARGS,
      "Output an 'warning' level entry"
   },
   {
      "MILE", pylog_mile, METH_VARARGS,
      "Output an 'mile' level entry"
   },
   {
      "INFO", pylog_info,  METH_VARARGS,
      "Output an 'info' level entry"
   },
   {
      "TRACE", pylog_trace, METH_VARARGS,
      "Output an 'trace' level entry"
   },
   {
      "DEBUG",  pylog_debug, METH_VARARGS,
      "Output an 'debug' level entry"
   },
   {
      "GETLEVEL", pylog_getlevel, METH_VARARGS,
      "Returns the current tracing level"
   },
   {
      "SETLEVEL", pylog_setlevel, METH_VARARGS,
      "Set the tracing level"
   },
   {
      "MASK", pylog_mask, METH_VARARGS,
      "Set which domains should be displayed"
   },
   {
      "NOTMASK", pylog_not_mask, METH_VARARGS,
      "Set which domains should be hidden"
   },
   {
      "GETMASKS", pylog_get_masks, METH_NOARGS,
      "Return a pair of lists with shown and hidden domains"
   },
   {
      "SETDOMAINLEVEL", pylog_set_domain_level,  METH_VARARGS,
      "Set per domain tracing level. Pass None to reset the domain filtering"
   },
   {
      "GETDOMAINLEVELS", pylog_get_domain_levels, METH_NOARGS,
      "Return a list of tuple with the domain and its level"
   },
   {NULL, NULL},
};


/* Module init function */
MOD_INIT(LOG)
{
   PyObject *m, *d;

   /* Init the module */
#if PY_MAJOR_VERSION >= 3
    static struct PyModuleDef moduledef =
    {
        PyModuleDef_HEAD_INIT,
        "LOG",                        /* m_name */
        "Fy built-in Logger module",  /* m_doc */
        -1,                           /* m_size */
        methods,                      /* m_methods */
        NULL,                /* m_reload */
        NULL,                /* m_traverse */
        NULL,                /* m_clear */
        NULL,                /* m_free */
    };

   m = PyModule_Create(&moduledef);
#else
   m = Py_InitModule("LOG", methods);
#endif

   PyModule_AddIntConstant( m, "LEVEL_ERROR", LOG_LEVEL_ERROR );
   PyModule_AddIntConstant( m, "LEVEL_WARN",  LOG_LEVEL_WARN );
   PyModule_AddIntConstant( m, "LEVEL_MILE",  LOG_LEVEL_MILE );
   PyModule_AddIntConstant( m, "LEVEL_INFO",  LOG_LEVEL_INFO );
   PyModule_AddIntConstant( m, "LEVEL_TRACE", LOG_LEVEL_TRACE );
   PyModule_AddIntConstant( m, "LEVEL_DEBUG", LOG_LEVEL_DEBUG );

   d = PyModule_GetDict(m);

   /* Init the local statics */
   s_class_method_name[0][0] = '\0';
   s_class_method_name[1][0] = '\0';

   /* initialize module variables/constants */
   log_error = PyErr_NewException("log.error", NULL, NULL);

   PyDict_SetItemString(d, "error", log_error);
   
   return MOD_SUCCESS_VAL(m);
}
