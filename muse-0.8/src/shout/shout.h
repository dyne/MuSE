/*  shout.h
**
**  api for libshout, the streaming library for icecast
*/
#ifndef __LIBSHOUT_SHOUT_H__
#define __LIBSHOUT_SHOUT_H__

#include <sys/types.h>

#define SHOUTERR_SUCCESS	(0)
#define SHOUTERR_INSANE		(1)
#define SHOUTERR_NOCONNECT	(2)
#define SHOUTERR_NOLOGIN	(3)
#define SHOUTERR_SOCKET		(4)
#define SHOUTERR_MALLOC		(5)
#define SHOUTERR_METADATA	(6)
#define SHOUTERR_CONNECTED	(7)
#define SHOUTERR_UNCONNECTED	(8)
#define SHOUTERR_UNSUPPORTED	(9)
#define SHOUTERR_REFUSED    (10)

#define SHOUT_FORMAT_VORBIS	(0)
#define SHOUT_FORMAT_MP3	(1)

#define SHOUT_PROTOCOL_ICE		(0)
#define SHOUT_PROTOCOL_XAUDIOCAST	(1)
#define SHOUT_PROTOCOL_ICY		(2)
#define SHOUT_PROTOCOL_HTTP	    (3)

typedef struct shout shout_t;
typedef struct shout_metadata shout_metadata_t;

#ifdef __cplusplus
extern "C" {
#endif

/* returns a static version string.  Non-null parameters will be set to the
 * value of the library major, minor, and patch levels, respectively */
const char *shout_version(int *major, int *minor, int *patch);

/* Allocates and sets up a new shout_t.  Returns NULL if it can't get enough
 * memory.  The returns shout_t must be disposed of with shout_free. */
shout_t *shout_new(void);

/* Free all memory allocated by a shout_t */
void shout_free(shout_t *self);

/* Returns a statically allocated string describing the last shout error
 * to occur.  Only valid until the next libshout call on this shout_t */
const char *shout_get_error(shout_t *self);

/* Return the error code (e.g. SHOUTERR_SOCKET) for this shout instance */
int shout_get_errno(shout_t *self);

/* Parameter manipulation functions.  libshout makes copies of all parameters,
 * the caller may free its copies after giving them to libshout.  May return
 * SHOUTERR_MALLOC */

int shout_set_host(shout_t *self, const char *host);
const char *shout_get_host(shout_t *self);

int shout_set_port(shout_t *self, unsigned short port);
unsigned short shout_get_port(shout_t *self);

int shout_set_password(shout_t *, const char *password);
const char *shout_get_password(shout_t *self);

int shout_set_mount(shout_t *self, const char *mount);
const char *shout_get_mount(shout_t *self);

int shout_set_name(shout_t *self, const char *name);
const char *shout_get_name(shout_t *self);

int shout_set_url(shout_t *self, const char *url);
const char *shout_get_url(shout_t *self);

int shout_set_genre(shout_t *self, const char *genre);
const char *shout_get_genre(shout_t *self);

int shout_set_user(shout_t *self, const char *username);
const char *shout_get_user(shout_t *self);

int shout_set_agent(shout_t *self, const char *username);
const char *shout_get_agent(shout_t *self);

int shout_set_description(shout_t *self, const char *description);
const char *shout_get_description(shout_t *self);

/* bitrate is in kbps */
int shout_set_bitrate(shout_t *self, unsigned int bitrate);
unsigned int shout_get_bitrate(shout_t *self);

/* takes a SHOUT_FORMAT_xxxx argument */
int shout_set_format(shout_t *self, unsigned int format);
unsigned int shout_get_format(shout_t *self);

/* takes a SHOUT_PROTOCOL_xxxxx argument */
int shout_set_protocol(shout_t *self, unsigned int protocol);
unsigned int shout_get_protocol(shout_t *self);

  int shout_set_public(shout_t *, unsigned int);
  unsigned int shout_get_public(shout_t *self);

/* Opens a connection to the server.  All parameters must already be set */
int shout_open(shout_t *self);

/* Closes a connection to the server */
int shout_close(shout_t *self);

/* Send data to the server, parsing it for format specific timing info */
int shout_send(shout_t *self, const unsigned char *data, size_t len);

/* Send unparsed data to the server.  Do not use this unless you know
 * what you are doing. 
 * Returns the number of bytes written, or < 0 on error.
 */
ssize_t shout_send_raw(shout_t *self, const unsigned char *data, size_t len);

/* Puts caller to sleep until it is time to send more data to the server */
void shout_sync(shout_t *self);

/* Sets MP3 metadata */
int shout_set_metadata(shout_t *self, shout_metadata_t *metadata);

/* Allocates a new metadata structure.  Must be freed by shout_metadata_free */
shout_metadata_t *shout_metadata_new(void);

/* Free resources allocated by shout_metadata_t */
void shout_metadata_free(shout_metadata_t *self);

/* Add a parameter to the metadata structure */
int shout_metadata_add(shout_metadata_t *self, const char *name, const char *value);

#ifdef __cplusplus
}
#endif

#endif /* __LIBSHOUT_SHOUT_H__ */
