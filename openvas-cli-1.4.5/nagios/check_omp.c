/* OMP Nagios Command Plugin
 * $Id$
 * Description: A nagios command plugin for the OpenVAS Management Protocol
 *
 * Authors:
 * Matthew Mundell <matthew.mundell@greenbone.net>
 * Michael Wiegand <michael.wiegand@greenbone.net>
 * Marcus Brinkmann <mb@g10code.com>
 * Werner Koch <wk@gnupg.org>
 *
 * Copyright:
 * Copyright (C) 2009-2016 Greenbone Networks GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/**
 * @file  check_omp.c
 * @brief The CHECK_OMP Nagios Command Plugin
 *
 * This command line tool provides command line arguments
 * corresponding to the OMP protocol commands for Nagios.
 */

/**
 * \mainpage
 * \section Introduction
 * \verbinclude README
 *
 * \section Installation
 * \verbinclude INSTALL
 *
 * \section copying License Information
 * \verbinclude COPYING
 */

#define _GNU_SOURCE

#include <assert.h>
#include <glib.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h> /* for exit() */
#include <string.h>
#include <stdarg.h>

#include <openvas/misc/openvas_server.h>
#ifdef _WIN32
#include <winsock2.h>
#endif
#ifndef _WIN32
#include <openvas/misc/openvas_logging.h>
#endif
#include <openvas/omp/omp.h>

/**
 * @brief The name of this program.
 */
#define OMP_PROGNAME "check_omp"

/**
 * @brief Default Manager (openvasmd) address.
 */
#define OPENVASMD_ADDRESS "127.0.0.1"

/**
 * @brief Default Manager port.
 */
#define OPENVASMD_PORT 9390


#define DEFAULT_SOCKET_TIMEOUT 10

/* See http://nagiosplug.sourceforge.net/developer-guidelines.html for the
 * official documentiation of the exit codes.
 * Note that we handle the conditions for the exit codes somewhat differently
 * since we are not directly monitoring a service, rather we are using a service
 * (OpenVAS Manager) to monitor another system.
 */
/**
 * @brief The plugin was able to contact the OpenVAS Manager. The returned
 *        results did not indicate a medium or high threat or an upward trend.
 */
#define NAGIOS_OK 0

/**
 * @brief The plugin was able to contact the OpenVAS Manager. The returned
 *        results did indicate a medium threat.
 */
#define NAGIOS_WARNING 1

/**
 * @brief The plugin was able to contact the OpenVAS Manager. The returned
 *        results did indicate a high threat or an upward trend.
 */
#define NAGIOS_CRITICAL 2

/**
 * @brief The plugin was not able to contact the OpenVAS Manager or was unable
 *        to parse the returned results. The response should include a
 *        diagnostic message.
 */
#define NAGIOS_UNKNOWN 3

#define NAGIOS_DEPENDENT 4


/* Type definitions.  */

/**
 * @brief Information needed to handle a connection to a server.
 */
typedef struct
{
  gnutls_session_t session;     ///< GnuTLS Session to use.
  int socket;                   ///< Socket to server.
  gchar *username;              ///< Username with which to connect.
  gchar *password;              ///< Password for user with which to connect.
  gchar *host_string;           ///< Server host string.
  gint port;                    ///< Port of server.
  gint timeout;                 ///< Timeout of request.
} server_connection_t;

/**
 * @brief Options for status display
 */
typedef struct
{
  gboolean report_link;         ///< TRUE if the report URL should be included
  gboolean dfn_ids;             ///< TRUE if DFN-CERT-IDs should be included
  gboolean oids;                ///< TRUE if NVT OIDs should be included
  gboolean descr;               ///< TRUE if NVT descriptions should be included
  gpointer manager_host;        ///< Pointer to name of the manager host for use in the report link
  gboolean log_messages;        ///< TRUE if log messages should be included
  gboolean scan_end;            ///< TRUE if the time the scan finished should be included
  guint autofp;                 ///< Whether to trust vendor security updates. 0 No, 1 full match, 2 partial.
  gint timeout;                 ///< Timeout of request.
  gboolean empty_as_unknown;    ///< TRUE if empty results should produce an UNKNOWN response instead of OK
} cmd_status_opts_t;

/* Prototypes.  */
static void
do_exit (int rc)
#if __GNUC__ >= 3
  __attribute__ ((__noreturn__));
#endif
;


/* Global options.  */

/* If this flag is set, UNKNOWN status codes are mapped to CRITICAL.  */
static int warnings_are_errors;

/* The value of the --overrides option.  */
static gint overrides_flag;

/* This flag is set if in any output a pipe symbol has been replaced
   by a broken bar (U+00A6).  Nagios uses the pipe symbol to separate
   performance data from the status. */
static int pipe_symbol_substituted;


/* Helper functions and macros.  */

static void
print_respond_string (const char *string)
{
  const char *s;

  for (s = string; *s; s++)
    {
      if (*s == '|')
        {
          fputs ("¦", stdout);
          pipe_symbol_substituted = 1;
        }
      else
        putchar (*s);
    }
}


/* Print the first respond line.  The return value is CODE, which is
   the Nagios plugin status code.  */
static int
respond (int code, const char *format, ...)
{
  va_list arg_ptr;
  char *buf;
  const char *status;

  switch (code)
    {
    case NAGIOS_OK:
      status = "OK";
      break;
    case NAGIOS_WARNING:
      status = "WARNING";
      break;
    case NAGIOS_CRITICAL:
      status = "CRITICAL";
      break;
    case NAGIOS_UNKNOWN:
      status = warnings_are_errors ? "CRITICAL" : "UNKNOWN";
      break;
    case NAGIOS_DEPENDENT:
      status = "DEPENDENT";
      break;
    default:
      fputs ("OMP UNKNOWN: Internal plugin error\n", stdout);
      return code;
    }

  va_start (arg_ptr, format);
  buf = g_strdup_vprintf (format, arg_ptr);
  va_end (arg_ptr);
  printf ("OMP %s: ", status);
  print_respond_string (buf);
  if (!*buf || buf[strlen (buf) - 1] != '\n')
    putchar ('\n');
  g_free (buf);
  return code;
}


/* Print more response lines.  This function does not allow to print
   performance data.  */
static void
respond_data (const char *format, ...)
{
  va_list arg_ptr;
  char *buf;

  va_start (arg_ptr, format);
  buf = g_strdup_vprintf (format, arg_ptr);
  va_end (arg_ptr);
  print_respond_string (buf);
  if (!*buf || buf[strlen (buf) - 1] != '\n')
    putchar ('\n');
  g_free (buf);
}

/* Print performance data.  */
static void
respond_perf_data (const char *format, ...)
{
  va_list arg_ptr;
  char *buf;

  if (pipe_symbol_substituted)
    fputs ("Note: pipe symbol(s) (U+007C) substituted"
           " by broken bar (U+00A6).\n", stdout);

  va_start (arg_ptr, format);
  buf = g_strdup_vprintf (format, arg_ptr);
  va_end (arg_ptr);
  g_print ("%s", buf);
  if (!*buf || buf[strlen (buf) - 1] != '\n')
    putchar ('\n');
  g_free (buf);
}

static void
do_exit (int rc)
{
  if (warnings_are_errors && rc == NAGIOS_UNKNOWN)
    rc = NAGIOS_CRITICAL;
  exit (rc);
}





/* Connection handling. */

/** @todo Return on fail. */
/**
 * @brief Connect to an openvas-manager, exiting on failure.
 *
 * @param[in]  connection   Struct containing data for the connection that
 *                          should be established.
 *
 * Exit with EXIT_FAILURE if connection could not be established or
 * authentication failed, printing a message to stderr.
 *
 * @return TRUE.  Does not return in fail case.
 */
static gboolean
manager_open (server_connection_t * connection)
{
  connection->socket =
    openvas_server_open (&connection->session, connection->host_string,
                         connection->port);

  if (connection->socket == -1)
    {
      do_exit (respond (NAGIOS_UNKNOWN, "Failed to acquire socket.\n"));
    }

  if (connection->username && connection->password)
    {
      omp_authenticate_info_opts_t authenticate_opts;
      gchar *timezone, *role, *severity, *pw_warning;

      authenticate_opts = omp_authenticate_info_opts_defaults;

      authenticate_opts.timeout = connection->timeout;
      authenticate_opts.username = connection->username;
      authenticate_opts.password = connection->password;
      authenticate_opts.timezone = &timezone;
      authenticate_opts.role = &role;
      authenticate_opts.severity = &severity;
      authenticate_opts.pw_warning = &pw_warning;

      switch (omp_authenticate_info_ext (&connection->session,
                                         authenticate_opts))
        {
        case 0:
          break;
        case 3:
          openvas_server_close (connection->socket, connection->session);
          do_exit (respond (NAGIOS_UNKNOWN,
                            "Timeout while trying to authenticate.\n"));
          break;
        default:
          openvas_server_close (connection->socket, connection->session);
          do_exit (respond (NAGIOS_UNKNOWN, "Failed to authenticate.\n"));
          break;
        }

      /* We currently have no need for the extra info, so free it. */
      g_free (*authenticate_opts.timezone);
      g_free (*authenticate_opts.role);
      g_free (*authenticate_opts.severity);
      g_free (*authenticate_opts.pw_warning);
    }

  return TRUE;
}

/**
 * @brief Closes the connection to a manager.
 *
 * @return 0 on success, -1 on failure.
 */
static int
manager_close (server_connection_t * server)
{
  return openvas_server_close (server->socket, server->session);
}


#define STATUS_BY_TREND 1
#define STATUS_BY_LAST_REPORT 2

static int
filter_report (entity_t report, const char *host_filter,
               cmd_status_opts_t status_opts)
{
  entity_t results;
  entities_t elems;
  entity_t elem;
  entity_t errors;
  int any_found = 0;
  int high_count = 0;
  int medium_count = 0;
  int low_count = 0;
  int log_count = 0;
  int error_count = 0;
  int response_code = NAGIOS_OK;
  GPtrArray *high_oids = NULL;
  GPtrArray *medium_oids = NULL;
  GPtrArray *low_oids = NULL;
  GPtrArray *log_oids = NULL;
  GPtrArray *high_names = NULL;
  GPtrArray *medium_names = NULL;
  GPtrArray *low_names = NULL;
  GPtrArray *log_names = NULL;
  GPtrArray *high_descriptions = NULL;
  GPtrArray *medium_descriptions = NULL;
  GPtrArray *low_descriptions = NULL;
  GPtrArray *log_descriptions = NULL;
  GPtrArray *high_dfn_ids_array = NULL;
  GPtrArray *medium_dfn_ids_array = NULL;
  GPtrArray *low_dfn_ids_array = NULL;
  GPtrArray *log_dfn_ids_array = NULL;
  GPtrArray *high_dfn_ids = NULL;
  GPtrArray *medium_dfn_ids = NULL;
  GPtrArray *low_dfn_ids = NULL;
  GPtrArray *log_dfn_ids = NULL;

  results = entity_child (report, "results");
  if (results == NULL)
    {
      return respond (NAGIOS_UNKNOWN, "Failed to get results list.\n");
    }

  if (status_opts.oids)
    {
      high_oids = g_ptr_array_new ();
      medium_oids = g_ptr_array_new ();
      low_oids = g_ptr_array_new ();
      log_oids = g_ptr_array_new ();
      high_names = g_ptr_array_new ();
      medium_names = g_ptr_array_new ();
      low_names = g_ptr_array_new ();
      log_names = g_ptr_array_new ();
      high_descriptions = g_ptr_array_new ();
      medium_descriptions = g_ptr_array_new ();
      low_descriptions = g_ptr_array_new ();
      log_descriptions = g_ptr_array_new ();
      high_dfn_ids = g_ptr_array_new ();
      medium_dfn_ids = g_ptr_array_new ();
      low_dfn_ids = g_ptr_array_new ();
      log_dfn_ids = g_ptr_array_new ();
    }
  else
    {
      /* Init to quiet compiler. */
      high_oids = medium_oids = low_oids = log_oids = NULL;
      high_names = medium_names = low_names = log_names = NULL;
      high_descriptions = medium_descriptions = NULL;
      low_descriptions = log_descriptions = NULL;
    }

  elems = results->entities;
  while ((elem = first_entity (elems)))
    {
      if (strcmp (entity_name (elem), "result") == 0)
        {
          entity_t entity;
          const char *host, *threat;

          entity = entity_child (elem, "host");
          if (entity == NULL)
            {
              return respond (NAGIOS_UNKNOWN,
                              "Failed to parse result host.\n");
            }
          host = entity_text (entity);

          /* Seeking to the right task...  */
          if (host_filter != NULL)
            if (strcmp (host, host_filter))
              goto skip_one_filter_report;
          any_found = 1;

          entity = entity_child (elem, "threat");
          if (entity == NULL)
            {
              return respond (NAGIOS_UNKNOWN,
                              "Failed to parse result threat.\n");
            }
          threat = entity_text (entity);
          if (!strcmp (threat, "High"))
            {
              high_count += 1;
              if (status_opts.oids)
                {
                  g_ptr_array_add (high_oids,
                                   (gpointer)
                                   entity_attribute (entity_child (elem, "nvt"),
                                                     "oid"));
                  g_ptr_array_add (high_names,
                                   (gpointer)
                                   entity_text (entity_child (entity_child (elem, "nvt"), "name")));
                  g_ptr_array_add (high_descriptions,
                                   (gpointer)
                                   entity_text (entity_child (elem, "description")));
                  if (status_opts.dfn_ids) {
                    high_dfn_ids_array = NULL;
                    entity_t cert_entity = entity_child (entity_child (elem, "nvt"), "cert");
                    entities_t cert_refs = cert_entity->entities;
                    if (cert_refs != NULL)
                      {
                        entity_t cert_ref;
                        while ((cert_ref = first_entity(cert_refs))) {
                          if (strcmp (entity_attribute (cert_ref, "type"), "DFN-CERT") == 0)
                            {
                              if (high_dfn_ids_array == NULL)
                                high_dfn_ids_array = g_ptr_array_new ();
                              g_ptr_array_add (high_dfn_ids_array,
                                               (gpointer)
                                               entity_attribute (cert_ref, "id"));
                            }
                          cert_refs = next_entities (cert_refs);
                        }
                        g_ptr_array_add (high_dfn_ids,
                                         (gpointer)
                                         high_dfn_ids_array);
                      }
                    else
                      {
                        g_ptr_array_add (high_dfn_ids, NULL);
                      }
                  }
                }
            }
          else if (!strcmp (threat, "Medium"))
            {
              medium_count += 1;
              if (status_opts.oids)
                {
                  g_ptr_array_add (medium_oids,
                                   (gpointer)
                                   entity_attribute (entity_child (elem, "nvt"),
                                                     "oid"));
                  g_ptr_array_add (medium_names,
                                   (gpointer)
                                   entity_text (entity_child (entity_child (elem, "nvt"), "name")));
                  g_ptr_array_add (medium_descriptions,
                                   (gpointer)
                                   entity_text (entity_child (elem, "description")));
                  if (status_opts.dfn_ids) {
                    medium_dfn_ids_array = NULL;
                    entity_t cert_entity = entity_child (entity_child (elem, "nvt"), "cert");
                    entities_t cert_refs = cert_entity->entities;
                    if (cert_refs != NULL)
                      {
                        entity_t cert_ref;
                        while ((cert_ref = first_entity(cert_refs))) {
                          if (strcmp (entity_attribute (cert_ref, "type"), "DFN-CERT") == 0)
                            {
                              if (medium_dfn_ids_array == NULL)
                                medium_dfn_ids_array = g_ptr_array_new ();
                              g_ptr_array_add (medium_dfn_ids_array,
                                               (gpointer)
                                               entity_attribute (cert_ref, "id"));
                            }
                          cert_refs = next_entities (cert_refs);
                        }
                        g_ptr_array_add (medium_dfn_ids,
                                         (gpointer)
                                         medium_dfn_ids_array);
                      }
                    else
                      {
                        g_ptr_array_add (medium_dfn_ids, NULL);
                      }
                  }
                }
            }
          else if (!strcmp (threat, "Low"))
            {
              low_count += 1;
              if (status_opts.oids)
                {
                  g_ptr_array_add (low_oids,
                                   (gpointer)
                                   entity_attribute (entity_child (elem, "nvt"),
                                                     "oid"));
                  g_ptr_array_add (low_names,
                                   (gpointer)
                                   entity_text (entity_child (entity_child (elem, "nvt"), "name")));
                  g_ptr_array_add (low_descriptions,
                                   (gpointer)
                                   entity_text (entity_child (elem, "description")));
                  if (status_opts.dfn_ids) {
                    low_dfn_ids_array = NULL;
                    entity_t cert_entity = entity_child (entity_child (elem, "nvt"), "cert");
                    entities_t cert_refs = cert_entity->entities;
                    if (cert_refs != NULL)
                      {
                        entity_t cert_ref;
                        while ((cert_ref = first_entity(cert_refs))) {
                          if (strcmp (entity_attribute (cert_ref, "type"), "DFN-CERT") == 0)
                            {
                              if (low_dfn_ids_array == NULL)
                                low_dfn_ids_array = g_ptr_array_new ();
                              g_ptr_array_add (low_dfn_ids_array,
                                               (gpointer)
                                               entity_attribute (cert_ref, "id"));
                            }
                          cert_refs = next_entities (cert_refs);
                        }
                        g_ptr_array_add (low_dfn_ids,
                                         (gpointer)
                                         low_dfn_ids_array);
                      }
                    else
                      {
                        g_ptr_array_add (low_dfn_ids, NULL);
                      }
                  }
                }
            }
          else if (!strcmp (threat, "Log"))
            {
              log_count += 1;
              if (status_opts.oids)
                {
                  g_ptr_array_add (log_oids,
                                   (gpointer)
                                   entity_attribute (entity_child (elem, "nvt"),
                                                     "oid"));
                  g_ptr_array_add (log_names,
                                   (gpointer)
                                   entity_text (entity_child (entity_child (elem, "nvt"), "name")));
                  g_ptr_array_add (log_descriptions,
                                   (gpointer)
                                   entity_text (entity_child (elem, "description")));
                  if (status_opts.dfn_ids) {
                    log_dfn_ids_array = NULL;
                    entity_t cert_entity = entity_child (entity_child (elem, "nvt"), "cert");
                    entities_t cert_refs = cert_entity->entities;
                    if (cert_refs != NULL)
                      {
                        entity_t cert_ref;
                        while ((cert_ref = first_entity(cert_refs))) {
                          if (strcmp (entity_attribute (cert_ref, "type"), "DFN-CERT") == 0)
                            {
                              if (log_dfn_ids_array == NULL)
                                log_dfn_ids_array = g_ptr_array_new ();
                              g_ptr_array_add (log_dfn_ids_array,
                                               (gpointer)
                                               entity_attribute (cert_ref, "id"));
                            }
                          cert_refs = next_entities (cert_refs);
                        }
                        g_ptr_array_add (log_dfn_ids,
                                         (gpointer)
                                         log_dfn_ids_array);
                      }
                    else
                      {
                        g_ptr_array_add (log_dfn_ids, NULL);
                      }
                  }
                }
            }
          else
            {
              return respond (NAGIOS_UNKNOWN, "Unknown result threat: %s.\n",
                              threat);
            }
        }
    skip_one_filter_report:
      elems = next_entities (elems);
    }

  errors = entity_child (report, "errors");
  if (errors != NULL)
    {
      if (host_filter)
        {
          entities_t error_elements;
          entity_t error_element;

          error_elements = errors->entities;
          while ((error_element = first_entity (error_elements)))
            {
              if (strcmp (entity_name (error_element), "error") == 0)
                {
                  entity_t error_host;

                  error_host = entity_child (error_element, "host");
                  if (strcmp (entity_text (error_host), host_filter) == 0)
                    error_count++;
                }
              error_elements = next_entities (error_elements);
            }
        }
      else
        {
          entity_t count_child;
          const char *error_text;

          count_child = entity_child (errors, "count");
          error_text = entity_text (count_child);
          error_count = atoi (error_text);
        }
    }

  if (high_count > 0)
    {
      response_code = NAGIOS_CRITICAL;
    }
  else if (medium_count > 0)
    {
      response_code = NAGIOS_WARNING;
    }

  if ((results->entities == NULL) || (!any_found && host_filter))
    {
      if (status_opts.empty_as_unknown)
        response_code = NAGIOS_UNKNOWN;
    }

  if ((error_count > 0) && (response_code == NAGIOS_OK))
    {
      response_code = NAGIOS_UNKNOWN;
    }

  respond (response_code, "%i vulnerabilities found - High: %i Medium: %i Low: %i\n",
           (high_count + medium_count + low_count), high_count, medium_count, low_count);

  if (results->entities == NULL)
    respond_data ("Report did not contain any vulnerabilities");
  else if (!any_found && host_filter)
    respond_data ("Report did not contain vulnerabilities for IP %s\n", host_filter);

  if (error_count > 0)
    {
      if (host_filter)
        {
          respond_data ("Report did contain %i errors for IP %s\n", error_count, host_filter);
        }
      else
        {
          respond_data ("Report did contain %i errors\n", error_count);
        }
    }

  if (status_opts.report_link)
    respond_data ("https://%s/omp?cmd=get_report&report_id=%s\n",
                  (gchar *) status_opts.manager_host, entity_attribute (report,
                                                                        "id"));

  if (status_opts.oids)
    {
      int i;
      unsigned int j;
      for (i = 0; i < high_count; i++)
        {
          respond_data ("NVT: %s (High) (%s)\n",
                        (char *) g_ptr_array_index (high_oids, i),
                        (char *) g_ptr_array_index (high_names, i));
          if (status_opts.descr)
            respond_data ("DESCR: %s\n",
                          (char *) g_ptr_array_index (high_descriptions, i));
          if (status_opts.dfn_ids && (g_ptr_array_index (high_dfn_ids, i) != NULL))
            {
              GPtrArray *dfn_ids_array = (GPtrArray *) g_ptr_array_index (high_dfn_ids, i);
              GString *dfn_ids_string = NULL;
              for (j = 0; j < dfn_ids_array->len; j++)
                {
                  if (dfn_ids_string == NULL)
                    {
                      dfn_ids_string = g_string_new ((char *) g_ptr_array_index (dfn_ids_array, j));
                    }
                  else
                    {
                      g_string_append (dfn_ids_string, (char *) g_ptr_array_index (dfn_ids_array, j));
                    }
                  if (j < dfn_ids_array->len - 1)
                    {
                      g_string_append (dfn_ids_string, ", ");
                    }
                }
              respond_data ("DFN-CERT: %s\n", dfn_ids_string->str);
              g_string_free (dfn_ids_string, FALSE);
            }
        }

      for (i = 0; i < medium_count; i++)
        {
          respond_data ("NVT: %s (Medium) (%s)\n",
                        (char *) g_ptr_array_index (medium_oids, i),
                        (char *) g_ptr_array_index (medium_names, i));
          if (status_opts.descr)
            respond_data ("DESCR: %s\n",
                          (char *) g_ptr_array_index (medium_descriptions, i));
          if (status_opts.dfn_ids && (g_ptr_array_index (medium_dfn_ids, i) != NULL))
            {
              GPtrArray *dfn_ids_array = (GPtrArray *) g_ptr_array_index (medium_dfn_ids, i);
              GString *dfn_ids_string = NULL;
              for (j = 0; j < dfn_ids_array->len; j++)
                {
                  if (dfn_ids_string == NULL)
                    {
                      dfn_ids_string = g_string_new ((char *) g_ptr_array_index (dfn_ids_array, j));
                    }
                  else
                    {
                      g_string_append (dfn_ids_string, (char *) g_ptr_array_index (dfn_ids_array, j));
                    }
                  if (j < dfn_ids_array->len - 1)
                    {
                      g_string_append (dfn_ids_string, ", ");
                    }
                }
              respond_data ("DFN-CERT: %s\n", dfn_ids_string->str);
              g_string_free (dfn_ids_string, FALSE);
            }
        }

      for (i = 0; i < low_count; i++)
        {
          respond_data ("NVT: %s (Low) (%s)\n",
                        (char *) g_ptr_array_index (low_oids, i),
                        (char *) g_ptr_array_index (low_names, i));
          if (status_opts.descr)
            respond_data ("DESCR: %s\n",
                          (char *) g_ptr_array_index (low_descriptions, i));
          if (status_opts.dfn_ids && (g_ptr_array_index (low_dfn_ids, i) != NULL))
            {
              GPtrArray *dfn_ids_array = (GPtrArray *) g_ptr_array_index (low_dfn_ids, i);
              GString *dfn_ids_string = NULL;
              for (j = 0; j < dfn_ids_array->len; j++)
                {
                  if (dfn_ids_string == NULL)
                    {
                      dfn_ids_string = g_string_new ((char *) g_ptr_array_index (dfn_ids_array, j));
                    }
                  else
                    {
                      g_string_append (dfn_ids_string, (char *) g_ptr_array_index (dfn_ids_array, j));
                    }
                  if (j < dfn_ids_array->len - 1)
                    {
                      g_string_append (dfn_ids_string, ", ");
                    }
                }
              respond_data ("DFN-CERT: %s\n", dfn_ids_string->str);
              g_string_free (dfn_ids_string, FALSE);
            }
        }

      if (status_opts.log_messages)
        for (i = 0; i < log_count; i++)
          {
            respond_data ("NVT: %s (Log) (%s)\n",
                          (char *) g_ptr_array_index (log_oids, i),
                          (char *) g_ptr_array_index (log_names, i));
            if (status_opts.descr)
              respond_data ("DESCR: %s\n",
                            (char *) g_ptr_array_index (log_descriptions, i));
            if (status_opts.dfn_ids && (g_ptr_array_index (log_dfn_ids, i) != NULL))
              {
                GPtrArray *dfn_ids_array = (GPtrArray *) g_ptr_array_index (log_dfn_ids, i);
                GString *dfn_ids_string = NULL;
                for (j = 0; j < dfn_ids_array->len; j++)
                  {
                    if (dfn_ids_string == NULL)
                      {
                        dfn_ids_string = g_string_new ((char *) g_ptr_array_index (dfn_ids_array, j));
                      }
                    else
                      {
                        g_string_append (dfn_ids_string, (char *) g_ptr_array_index (dfn_ids_array, j));
                      }
                    if (j < dfn_ids_array->len - 1)
                      {
                        g_string_append (dfn_ids_string, ", ");
                      }
                  }
                respond_data ("DFN-CERT: %s\n", dfn_ids_string->str);
                g_string_free (dfn_ids_string, FALSE);
              }
          }

      g_ptr_array_free (high_oids, TRUE);
      g_ptr_array_free (medium_oids, TRUE);
      g_ptr_array_free (low_oids, TRUE);
      g_ptr_array_free (log_oids, TRUE);
      g_ptr_array_free (high_names, TRUE);
      g_ptr_array_free (medium_names, TRUE);
      g_ptr_array_free (low_names, TRUE);
      g_ptr_array_free (log_names, TRUE);
      g_ptr_array_free (high_descriptions, TRUE);
      g_ptr_array_free (medium_descriptions, TRUE);
      g_ptr_array_free (low_descriptions, TRUE);
      g_ptr_array_free (log_descriptions, TRUE);
      g_ptr_array_free (high_dfn_ids, TRUE);
      g_ptr_array_free (medium_dfn_ids, TRUE);
      g_ptr_array_free (low_dfn_ids, TRUE);
      g_ptr_array_free (log_dfn_ids, TRUE);
    }

  if (status_opts.scan_end)
    respond_data ("SCAN_END: %s\n", entity_text (entity_child (report, "scan_end")));

  respond_perf_data ("|High=%i Medium=%i Low=%i\n",
                     high_count, medium_count, low_count);
  return response_code;
}

/* If host_filter is not NULL, mode must be STATUS_BY_LAST_REPORT and
   host_filter is a string specifying for which IP the last results
   are returned.
   It is assumed that the "tasks" only contains a single task with the
   name "task_filter" or no task at all.
*/
static int
cmd_status_impl (server_connection_t * connection, const char *task_filter,
                 entities_t tasks, int mode, char *host_filter,
                 cmd_status_opts_t status_opts)
{
  entity_t task;
  while ((task = first_entity (tasks)))
    {
      if (strcmp (entity_name (task), "task") == 0)
        {
          entity_t entity, report;

          /* FIXME: Check status (Done vs Requested)  */

          if (mode == STATUS_BY_TREND)
            {
              const char *trend;
              entity = entity_child (task, "trend");
              if (entity == NULL)
                return respond (NAGIOS_UNKNOWN,
                                "Failed to parse task trend.\n");

              trend = entity_text (entity);

              if (!strcmp (trend, "up") || !strcmp (trend, "more"))
                {
                  return respond (NAGIOS_CRITICAL, "Trend is %s\n", trend);
                }
              else if (!strcmp (trend, "down") || !strcmp (trend, "same")
                       || !strcmp (trend, "less"))
                {
                  return respond (NAGIOS_OK, "Trend is %s\n", trend);
                }
              else if (!strcmp (trend, ""))
                {
                  return respond (NAGIOS_UNKNOWN, "Trend is not available\n");
                }
              else
                {
                  return (respond (NAGIOS_UNKNOWN, "Trend is unknown: %s\n", trend));
                }
            }
          else
            {
              /* STATUS_BY_LAST_REPORT */
              entity_t full_report;
              omp_get_report_opts_t opts = omp_get_report_opts_defaults;

              report = entity_child (task, "last_report");
              if (report == NULL)
                return respond (NAGIOS_UNKNOWN, "Report is not available\n");

              report = entity_child (report, "report");
              if (report == NULL)
                return respond (NAGIOS_UNKNOWN,
                                "Failed to parse last_report\n");

              opts.report_id = entity_attribute (report, "id");
              if (opts.report_id == NULL)
                {
                  return respond (NAGIOS_UNKNOWN,
                                  "Failed to parse last_report's "
                                  "report ID.\n");
                }

              opts.apply_overrides = overrides_flag;
              opts.autofp = status_opts.autofp;
              opts.timeout = status_opts.timeout;

              switch (omp_get_report_ext (&(connection->session), opts, &full_report))
                {
                case 0:
                  break;
                case 2:
                  return respond (NAGIOS_UNKNOWN,
                                  "Timeout while getting full report.\n");
                default:
                  return respond (NAGIOS_UNKNOWN,
                                  "Failed to get full report.\n");
                }

              full_report = entity_child (full_report, "report");
              if (full_report == NULL)
                return respond (NAGIOS_UNKNOWN,
                                "Failed to get first full report wrapper\n");

              full_report = entity_child (full_report, "report");
              if (full_report == NULL)
                return respond (NAGIOS_UNKNOWN,
                                "Failed to get first full report\n");

              return filter_report (full_report, host_filter, status_opts);

              /* FIXME: Maybe add check here if the report is too
                 old?  */
            }

          /* Never reached.  */
          return respond (NAGIOS_UNKNOWN, "Internal error\n");
        }
      tasks = next_entities (tasks);
    }

  return respond (NAGIOS_UNKNOWN, "Unknown task: %s\n", task_filter);
}


/* Entry point. */

int
main (int argc, char **argv)
{
  server_connection_t *connection = NULL;
  /* The return status of the command. */
  int exit_status = -1;

  /* Global options. */
  static gboolean print_version = FALSE;
  static gboolean be_verbose = FALSE;
  static gchar *manager_host_string = NULL;
  static gint manager_port = OPENVASMD_PORT;
  static gchar *omp_username = NULL;
  static gchar *omp_password = NULL;
  /* Command get-omp-version. */
  static gboolean cmd_ping = FALSE;
  static gint timeout = DEFAULT_SOCKET_TIMEOUT;
  static gboolean cmd_status = FALSE;
  static gboolean status_trend = FALSE;
  static gboolean status_last_report = FALSE;
  static gchar *task_string = NULL;
  static gchar *host_filter = NULL;
  static gboolean connection_details = FALSE;
  static gboolean report_link = FALSE;
  static gboolean display_dfn_ids = FALSE;
  static gboolean display_oids = FALSE;
  static gboolean display_descriptions = FALSE;
  static gboolean display_log_messages = FALSE;
  static gboolean display_scan_end = FALSE;
  static guint autofp = 0;
  static gboolean empty_as_unknown = FALSE;
  static gboolean use_asset_management = FALSE;
  /* The rest of the args. */
  static gchar **rest = NULL;

  GError *error = NULL;

  GOptionContext *option_context;
  static GOptionEntry option_entries[] = {
    /* Global options. */
    {"host", 'H', 0, G_OPTION_ARG_STRING, &manager_host_string,
     "Connect to manager on host <host>", "<host>"},
    {"port", 'p', 0, G_OPTION_ARG_INT, &manager_port,
     "Use port number <number>", "<number>"},
    {"version", 'V', 0, G_OPTION_ARG_NONE, &print_version,
     "Print version.", NULL},
    {"verbose", 'v', 0, G_OPTION_ARG_NONE, &be_verbose,
     "Verbose messages (WARNING: may reveal passwords).", NULL},
    {"Werror", 0, 0, G_OPTION_ARG_NONE, &warnings_are_errors,
     "Turn status UNKNOWN into status CRITICIAL.", NULL},
    {"username", 'u', 0, G_OPTION_ARG_STRING, &omp_username,
     "OMP username", "<username>"},
    {"password", 'w', 0, G_OPTION_ARG_STRING, &omp_password,
     "OMP password", "<password>"},
    {"ping", 'O', 0, G_OPTION_ARG_NONE, &cmd_ping,
     "Ping the manager", NULL},
    {"timeout", 't', 0, G_OPTION_ARG_INT, &timeout,
     "Wait <seconds> for response (0 disables timeout)", "<seconds>"},
    /* @todo "ping-timeout" remains a hidden synonym for "timout" for backward
     * compatibility. Can be removed for version >= 1.5.
     */
    {"ping-timeout", 0, G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_INT, &timeout,
     NULL, NULL},
    {"status", 0, 0, G_OPTION_ARG_NONE, &cmd_status,
     "Report status of task", NULL},
    {"trend", 0, 0, G_OPTION_ARG_NONE, &status_trend,
     "Report status by trend (default)", NULL},
    {"last-report", 0, 0, G_OPTION_ARG_NONE, &status_last_report,
     "Report status by last report", NULL},
    {"task", 'T', 0, G_OPTION_ARG_STRING, &task_string,
     "Report status of task <task>", "<task>"},
    {"host-filter", 'F', 0, G_OPTION_ARG_STRING, &host_filter,
     "Report last report status of host <ip>", "<ip>"},
    {"overrides", 0, 0, G_OPTION_ARG_INT, &overrides_flag,
     "Include overrides (N: 0=no, 1=yes)", "N"},
    {"details", 'd', 0, G_OPTION_ARG_NONE, &connection_details,
     "Include connection details in output", NULL},
    {"report-link", 'l', 0, G_OPTION_ARG_NONE, &report_link,
     "Include URL of report in output", NULL},
    {"dfn", 0, 0, G_OPTION_ARG_NONE, &display_dfn_ids,
     "Include DFN-CERT IDs on vulnerabilities in output", NULL},
    {"oid", 0, 0, G_OPTION_ARG_NONE, &display_oids,
     "Include OIDs of NVTs finding vulnerabilities in output", NULL},
    {"descr", 0, 0, G_OPTION_ARG_NONE, &display_descriptions,
     "Include descriptions of NVTs finding vulnerabilities in output", NULL},
    {"showlog", 0, 0, G_OPTION_ARG_NONE, &display_log_messages,
     "Include log messages in output", NULL},
    {"scanend", 0, 0, G_OPTION_ARG_NONE, &display_scan_end,
     "Include timestamp of scan end in output", NULL},
    {"autofp", 0, 0, G_OPTION_ARG_INT, &autofp,
     "Trust vendor security updates for automatic false positive filtering (0=No, 1=full match, 2=partial).", "<n>"},
    {"empty-as-unknown", 'e', 0, G_OPTION_ARG_NONE, &empty_as_unknown,
     "Respond with UNKNOWN on empty results", NULL},
    {"use-asset-management", 'A', 0, G_OPTION_ARG_NONE, &use_asset_management,
     "Request host status via Asset Management", NULL},
    {G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &rest,
     NULL, NULL},
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

  if (setlocale (LC_ALL, "") == NULL)
    {
      do_exit (respond (NAGIOS_UNKNOWN, "Failed to setlocale\n\n"));
    }

  option_context =
    g_option_context_new ("- OpenVAS OMP Command Line Interface");
  g_option_context_add_main_entries (option_context, option_entries, NULL);
  if (!g_option_context_parse (option_context, &argc, &argv, &error))
    {
      printf ("%s\n\n", error->message);
      do_exit (NAGIOS_UNKNOWN);
    }
  g_option_context_free (option_context);

  if (print_version)
    {
      printf ("Check-OMP Nagios Command Plugin %s\n", OPENVASCLI_VERSION);
      printf ("Copyright (C) 2016 Greenbone Networks GmbH\n");
      printf ("License GPLv2+: GNU GPL version 2 or later\n");
      printf
        ("This is free software: you are free to change and redistribute it.\n"
         "There is NO WARRANTY, to the extent permitted by law.\n\n");
      do_exit (EXIT_SUCCESS);
    }

  /* Check that one and at most one command option is present. */
  {
    int commands;
    commands = (int) cmd_ping + (int) cmd_status;
    if (commands == 0)
      {
        respond (NAGIOS_UNKNOWN, "One command option must be present.\n");
        do_exit (NAGIOS_UNKNOWN);
      }
    if (commands > 1)
      {
        respond (NAGIOS_UNKNOWN, "Only one command option must be present.\n");
        do_exit (NAGIOS_UNKNOWN);
      }
  }

  /* Set defaults.  */
  if (!status_trend && !status_last_report)
    status_trend = TRUE;
  if (status_trend && status_last_report)
    {
      respond (NAGIOS_UNKNOWN, "--trend and --last-report are exclusive.\n");
      do_exit (NAGIOS_UNKNOWN);
    }


  /* Setup the connection structure.  */
  connection = g_malloc0 (sizeof (*connection));

  if (manager_host_string != NULL)
    connection->host_string = manager_host_string;
  else
    connection->host_string = OPENVASMD_ADDRESS;

  if (manager_port <= 0 || manager_port >= 65536)
    {
      respond (NAGIOS_UNKNOWN,
               "Manager port must be a number between 0 and 65536.\n");
      do_exit (NAGIOS_UNKNOWN);
    }

  connection->port = manager_port;

  if (omp_username != NULL)
    connection->username = omp_username;
  if (omp_password != NULL)
    connection->password = omp_password;

  if (timeout < 0)
    {
      respond (NAGIOS_UNKNOWN,
               "Timeout must be a non-negative number.\n");
      do_exit (NAGIOS_UNKNOWN);
    }

  connection->timeout = timeout;

  if (be_verbose)
    {
      /** @todo Other modules ship with log level set to warning. */
      fprintf (stderr, "Will try to connect to host %s, port %d...\n",
               connection->host_string, connection->port);
    }
  else
    {
#ifndef _WIN32
      g_log_set_default_handler (openvas_log_silent, NULL);
#endif
    }

  /* Run the single command. */

  if (cmd_ping)
    {
      int res;
      manager_open (connection);
      /* Returns 0 on success, 1 if manager closed connection, 2 on
         timeout, -1 on error */
      res = omp_ping (&(connection->session), connection->timeout);
      if (res == 0)
        {
          exit_status = respond (NAGIOS_OK, "Alive and kicking!\n");
        }
      else if (res == 1)
        {
          exit_status = respond (NAGIOS_CRITICAL, "Connection closed\n");
        }
      else if (res == 2)
        {
          exit_status = respond (NAGIOS_CRITICAL, "Connection timed out\n");
        }
      else
        {
          exit_status = respond (NAGIOS_CRITICAL, "Unknown error\n");
        }
      manager_close (connection);
    }
  else if (cmd_status)
    {
      entity_t status;

      if (use_asset_management)
        {
          if (host_filter == NULL)
            {
              exit_status =
                respond (NAGIOS_UNKNOWN, "Status request via Asset Management requires host filter\n");
            }
          else
            {
              entity_t asset_report;
              entity_t host_detail;
              entities_t host_details;
              gchar *report_id = NULL;
              entity_t full_report;
              cmd_status_opts_t status_opts;

              int res;
              int high_count = 0;
              int medium_count = 0;
              int low_count = 0;

              omp_get_report_opts_t asset_opts = omp_get_report_opts_defaults;
              omp_get_report_opts_t report_opts = omp_get_report_opts_defaults;

              asset_opts.overrides = overrides_flag;
              asset_opts.autofp = autofp;
              asset_opts.timeout = timeout;
              asset_opts.type = "assets";
              asset_opts.host = host_filter;

              manager_open (connection);
              res = omp_get_report_ext (&(connection->session), asset_opts, &asset_report);
              if (res == 0)
                {
                  asset_report = entity_child (asset_report, "report");
                  if (asset_report == NULL)
                    {
                      exit_status = respond (NAGIOS_UNKNOWN, "Failed to get first asset report wrapper\n");
                    }
                  else
                    {
                      asset_report = entity_child (asset_report, "report");
                      if (asset_report == NULL)
                        {
                          exit_status =  respond (NAGIOS_UNKNOWN, "Failed to get first asset report\n");
                        }
                      else
                        {
                          asset_report = entity_child (asset_report, "host");
                          if (asset_report == NULL)
                            {
                              exit_status =  respond (NAGIOS_UNKNOWN, "Failed to get asset host element\n");
                            }
                          else
                            {
                              host_details = asset_report->entities;
                              while ((host_detail = first_entity (host_details)))
                                {
                                  if (strcmp (entity_name (host_detail), "detail") == 0)
                                    {
                                      entity_t name;
                                      entity_t value;

                                      name = entity_child (host_detail, "name");
                                      value = entity_child (host_detail, "value");

                                      if (strcmp (entity_text (name), "report/@id") == 0)
                                        report_id = g_strdup (entity_text (value));
                                      if (strcmp (entity_text (name), "report/result_count/high") == 0)
                                        high_count = atoi (entity_text (value));
                                      if (strcmp (entity_text (name), "report/result_count/medium") == 0)
                                        medium_count = atoi (entity_text (value));
                                      if (strcmp (entity_text (name), "report/result_count/low") == 0)
                                        low_count = atoi (entity_text (value));
                                    }
                                  host_details = next_entities (host_details);
                                }

                              if (report_id == NULL)
                                {
                                  exit_status = respond (NAGIOS_UNKNOWN, "Failed to get report_id via Asset Management\n");
                                }
                              else
                                {
                                  if ((high_count + medium_count) == 0)
                                    {
                                      int response_code = NAGIOS_OK;
                                      exit_status = respond (response_code,
                                                             "%i vulnerabilities found - High: 0 Medium: 0 Low: %i\n",
                                                             low_count, low_count);

                                      if (report_link)
                                        respond_data ("https://%s/omp?cmd=get_report&report_id=%s\n",
                                                      (gchar *) (gpointer) connection->host_string, report_id);

                                      if (display_scan_end)
                                        respond_data ("SCAN_END: %s\n", entity_text (entity_child (asset_report, "end")));

                                      respond_perf_data ("|High=%i Medium=%i Low=%i\n",
                                                         high_count, medium_count, low_count);
                                    }
                                  else
                                    {
                                      report_opts.report_id = report_id;

                                      status_opts.report_link = report_link;
                                      status_opts.dfn_ids = display_dfn_ids;
                                      status_opts.oids = display_oids;
                                      status_opts.manager_host = (gpointer) connection->host_string;
                                      status_opts.descr = display_descriptions;
                                      status_opts.log_messages = display_log_messages;
                                      status_opts.scan_end = display_scan_end;
                                      status_opts.autofp = autofp;
                                      status_opts.timeout = timeout;
                                      status_opts.empty_as_unknown = empty_as_unknown;

                                      report_opts.apply_overrides = overrides_flag;
                                      report_opts.autofp = status_opts.autofp;
                                      report_opts.timeout = status_opts.timeout;

                                      if (!display_log_messages)
                                        report_opts.levels = "hml";

                                      res = omp_get_report_ext (&(connection->session), report_opts, &full_report);
                                      if (res == 0)
                                        {
                                          full_report = entity_child (full_report, "report");
                                          if (full_report == NULL)
                                            {
                                              exit_status = respond (NAGIOS_UNKNOWN,
                                                                     "Failed to get first full report wrapper\n");
                                            }
                                          else
                                            {
                                              full_report = entity_child (full_report, "report");
                                              if (full_report == NULL)
                                                {
                                                  exit_status = respond (NAGIOS_UNKNOWN,
                                                                         "Failed to get first full report\n");
                                                }
                                              else
                                                {

                                                  exit_status = filter_report (full_report, host_filter, status_opts);
                                                }
                                            }
                                        }
                                      else if (res == 2)
                                        {
                                          exit_status = respond (NAGIOS_UNKNOWN,
                                                                 "Timeout while getting full report.\n");
                                        }
                                      else
                                        {
                                          exit_status = respond (NAGIOS_UNKNOWN,
                                                                 "Failed to get full report.\n");
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
              else if (res == 2)
                {
                  exit_status = respond (NAGIOS_UNKNOWN, "Timeout while getting asset report.\n");
                }
              else
                {
                  exit_status = respond (NAGIOS_UNKNOWN, "Failed to get asset report.\n");
                }
            }
        }
      else if (task_string == NULL)
        {
          exit_status =
            respond (NAGIOS_UNKNOWN, "Status request requires task name\n");
        }
      else
        {
          manager_open (connection);
          omp_get_tasks_opts_t opts;
          cmd_status_opts_t status_opts;

          opts = omp_get_tasks_opts_defaults;
          opts.details = 1;
          /* TODO: Needs to be free'd at some point */
          opts.filter = g_strdup_printf ("permission=any owner=any rows=1 name=\"%s\"", task_string);
          opts.timeout = timeout;

          if (display_descriptions)
            display_oids = TRUE;

          if (display_dfn_ids)
            display_oids = TRUE;

          status_opts.report_link = report_link;
          status_opts.dfn_ids = display_dfn_ids;
          status_opts.oids = display_oids;
          status_opts.manager_host = (gpointer) connection->host_string;
          status_opts.descr = display_descriptions;
          status_opts.log_messages = display_log_messages;
          status_opts.scan_end = display_scan_end;
          status_opts.autofp = autofp;
          status_opts.timeout = timeout;
          status_opts.empty_as_unknown = empty_as_unknown;

          /* Returns 0 on success, 2 on timeout, -1 or OMP code on error.  */
          switch (omp_get_tasks_ext (&(connection->session), opts, &status))
            {
            case 0:
              exit_status =
                cmd_status_impl (connection, task_string, status->entities,
                                 status_trend ? STATUS_BY_TREND :
                                 STATUS_BY_LAST_REPORT, host_filter,
                                 status_opts);
              break;
            case 2:
              exit_status = respond (NAGIOS_UNKNOWN, "Timeout while getting tasks\n");
              break;
            default:
              exit_status = respond (NAGIOS_UNKNOWN, "Get tasks failed\n");
              break;
            }

          manager_close (connection);
        }
    }
  else
    /* The option processing ensures that at least one command is present. */
    assert (0);

  /* Exit. */

  if (connection_details)
    {
      if (connection->host_string)
        respond_data ("GSM_Host: %s:%d\n", connection->host_string,
                      (int) connection->port);
      if (connection->username)
        respond_data ("OMP_User: %s\n", connection->username);
      if (task_string && cmd_status)
        respond_data ("Task: %s\n", task_string);
    }

  if (be_verbose)
    {
      if (exit_status != NAGIOS_OK)
        respond_data ("Command failed.\n");
      else
        respond_data ("Command completed successfully.\n");
    }

  do_exit (exit_status);
}