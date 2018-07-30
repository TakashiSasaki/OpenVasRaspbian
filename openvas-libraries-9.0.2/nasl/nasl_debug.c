/* Nessus Attack Scripting Language
 *
 * Copyright (C) 2002 - 2004 Tenable Network Security
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2,
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <stdarg.h>
#include <unistd.h>

#include "../misc/arglists.h"

#include "nasl_tree.h"
#include "nasl_global_ctxt.h"
#include "nasl_func.h"
#include "nasl_var.h"
#include "nasl_lex_ctxt.h"
#include "exec.h"
#include "../misc/plugutils.h"
#include "../misc/openvas_logging.h"


extern FILE *nasl_trace_fp;

static char *debug_filename = NULL;
static char *debug_funname = NULL;

static GHashTable *functions_filenames = NULL;

const char *
nasl_get_filename (const char *function)
{
  char *ret = NULL;

  if (functions_filenames && function)
    ret = g_hash_table_lookup (functions_filenames, function);
  return ret ?: debug_filename;
}

/* For debug purposes, the non internal function name is saved to
 * be displayed in the error message.
 */
void
nasl_set_function_name (const char *funname)
{
  if (funname == debug_funname)
    return;
  g_free (debug_funname);
  debug_funname = g_strdup (funname);
}

const char *
nasl_get_function_name ()
{
  return debug_funname;
}

void
nasl_set_filename (const char *filename)
{
  assert (filename);
 
  if (filename == debug_filename)
    return;
  g_free (debug_filename);
  debug_filename = g_strdup (filename);
}
 
void
nasl_set_function_filename (const char *function)
{
  assert (function);
  
  if (!functions_filenames)
    functions_filenames = g_hash_table_new_full
      (g_str_hash, g_str_equal, g_free, g_free);
  g_hash_table_insert (functions_filenames, g_strdup (function),
                       g_strdup (debug_filename));
}

void
nasl_perror (lex_ctxt * lexic, char *msg, ...)
{
  va_list param;
  gchar debug_message[4096];
  gchar *final_message;
  char *script_name = "";
  lex_ctxt *lexic2 = NULL;
  int line_nb = 0;

  va_start (param, msg);

  if (lexic != NULL)
    {
      script_name = arg_get_value (lexic->script_infos, "script_name");
      if (script_name == NULL)
        script_name = "";
      /* Climbing up to find a line number */
      for (lexic2 = lexic; lexic2 != NULL; lexic2 = lexic2->up_ctxt)
        {
          if (lexic2->line_nb != 0)
            {
              line_nb = lexic2->line_nb;
              break;
            }
        }
    }

  g_vsnprintf (debug_message, sizeof (debug_message), msg, param);
  if ((debug_funname != NULL) && (g_strcmp0 (debug_funname, "") != 0))
    final_message = g_strconcat ("In function '", debug_funname,
                                 "()': ", debug_message, NULL);
  else
    final_message = g_strdup (debug_message);
  
  if (g_strcmp0 (debug_filename, script_name) == 0)
    log_legacy_write ("[%d](%s:%d) %s\n", getpid (), script_name,
                      line_nb, final_message);
  else
    log_legacy_write ("[%d](%s)(%s:%d) %s\n", getpid (), script_name,
                      debug_filename, line_nb, final_message);
  g_free(final_message);
  
  /** @todo Enable this when the NVTs are ready.  Sends ERRMSG to client. */
#if 0
  if (lexic != NULL)
    post_error (lexic->script_infos, 1, debug_message);
#endif

  va_end (param);
}

/**
 * @brief Checks if the nasl_trace_fp is set.
 *
 * @return 0 if nasl_trace_fp == NULL, 1 otherwise.
 */
int
nasl_trace_enabled (void)
{
  if (nasl_trace_fp == NULL)
    return 0;
  else
    return 1;
}

/**
 * @brief Prints debug message in printf fashion to nasl_trace_fp if it exists.
 *
 * Like @ref nasl_perror, but to the nasl_trace_fp.
 */
void
nasl_trace (lex_ctxt * lexic, char *msg, ...)
{
  va_list param;
  char debug_message[4096];
  char *script_name = "", *p;

  if (nasl_trace_fp == NULL)
    return;
  va_start (param, msg);

  if (lexic != NULL)
    {
      script_name = arg_get_value (lexic->script_infos, "script_name");
      if (script_name == NULL)
        script_name = "";
    }

  vsnprintf (debug_message, sizeof (debug_message), msg, param);
  for (p = debug_message; *p != '\0'; p++)
    ;
  if (p == debug_message || p[-1] != '\n')
    fprintf (nasl_trace_fp, "[%d](%s) %s\n", getpid (), script_name,
             debug_message);
  else
    fprintf (nasl_trace_fp, "[%d](%s) %s", getpid (), script_name,
             debug_message);

  va_end (param);
}
