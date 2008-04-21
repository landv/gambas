
/* #ifdef LIBSMTP_USE_MIME */

  #ifndef __G_LIB_H__
    #include <glib.h>
  #endif

  #ifndef LIB_SMTP_MIME_H

    #define LIB_SMTP_MIME_H

    /* MIME types */

    #define LIBSMTP_MIME_TEXT	0
    #define LIBSMTP_MIME_MESSAGE	1
    #define LIBSMTP_MIME_IMAGE	2
    #define LIBSMTP_MIME_AUDIO	3
    #define LIBSMTP_MIME_VIDEO	4
    #define LIBSMTP_MIME_APPLICATION	5
    #define LIBSMTP_MIME_MULTIPART	6
    #define LIBSMTP_MIME_CUSTOM	7

    #define LIBSMTP_MAX_MIME	7


    /* MIME subtypes */

    /* 0 to 999 are TEXT subtypes */

    #define LIBSMTP_MIME_SUB_PLAIN	0
    #define LIBSMTP_MIME_SUB_HTML	1
    #define LIBSMTP_MIME_SUB_ENGLISH	2
    #define LIBSMTP_MIME_SUB_RICHTEXT	3

    #define LIBSMTP_MAX_MIME_SUB0	3


    /* 1000 to 1999 are MESSAGE subtypes */

    #define LIBSMTP_MIME_SUB_RFC822	1000
    #define LIBSMTP_MIME_SUB_PARTIAL	1001

    #define LIBSMTP_MAX_MIME_SUB1	1001


    /* 2000 to 2999 are IMAGE subtypes */

    #define LIBSMTP_MIME_SUB_GIF	2000
    #define LIBSMTP_MIME_SUB_JPG	2001
    #define LIBSMTP_MIME_SUB_PNG	2002
    #define LIBSMTP_MIME_SUB_TIFF	2003
    #define LIBSMTP_MIME_SUB_MS_BMP	2004
    #define LIBSMTP_MIME_SUB_XBITMAP	2005
    #define LIBSMTP_MIME_SUB_XPIXMAP	2006
    #define LIBSMTP_MIME_SUB_PORTABLE_ANYMAP	2007
    #define LIBSMTP_MIME_SUB_PORTABLE_BITMAP	2008
    #define LIBSMTP_MIME_SUB_PORTABLE_GRAYMAP	2009
    #define LIBSMTP_MIME_SUB_PORTABLE_PIXMAP	2010

    #define LIBSMTP_MAX_MIME_SUB2	2010


    /* 3000 to 3999 are AUDIO subtypes */

    #define LIBSMTP_MIME_SUB_MPEGAUD	3000
    #define LIBSMTP_MIME_SUB_MIDI	3001
    #define LIBSMTP_MIME_SUB_WAV	3002
    #define LIBSMTP_MIME_SUB_AIFF	3003

    #define LIBSMTP_MAX_MIME_SUB3	3003


    /* 4000 to 4999 are VIDEO subtypes */

    #define LIBSMTP_MIME_SUB_MPEGVID	4000
    #define LIBSMTP_MIME_SUB_MSVIDEO	4001
    #define LIBSMTP_MIME_SUB_QUICKTIME	4002
    #define LIBSMTP_MIME_SUB_FLI	4003

    #define LIBSMTP_MAX_MIME_SUB4	4003


    /* 5000 to 5999 are APPLICATION subtypes */

    #define LIBSMTP_MIME_SUB_RTF	5000
    #define LIBSMTP_MIME_SUB_POSTSCRIPT	5001
    #define LIBSMTP_MIME_SUB_PDF	5002
    #define LIBSMTP_MIME_SUB_ZIP	5003
    #define LIBSMTP_MIME_SUB_DEBIAN_PACKAGE	5004
    #define LIBSMTP_MIME_SUB_EXECUTABLE	5005
    #define LIBSMTP_MIME_SUB_GTAR	5006
    #define LIBSMTP_MIME_SUB_SHELLSCRIPT	5007
    #define LIBSMTP_MIME_SUB_TAR	5008
    #define LIBSMTP_MIME_SUB_OCTET_STREAM	5009

    #define LIBSMTP_MAX_MIME_SUB5	5008


    /* 6000 to 6999 are MULTIPART subtypes */

    #define LIBSMTP_MIME_SUB_MIXED	6000
    #define LIBSMTP_MIME_SUB_PARALLEL	6001
    #define LIBSMTP_MIME_SUB_DIGEST	6002
    #define LIBSMTP_MIME_SUB_ALTERNATIVE	6003

    #define LIBSMTP_MAX_MIME_SUB6	6003


    /* 30000 (for signed ints!!) is the CUSTOM subtype */

    #define LIBSMTP_MIME_SUB_CUSTOM	30000


    /* Encoding types */

    #define LIBSMTP_ENC_7BIT	0
    #define LIBSMTP_ENC_8BIT	1 /* not really used in current version */
    #define LIBSMTP_ENC_BINARY	2 /* not really used in current version */
    #define LIBSMTP_ENC_BASE64	3
    #define LIBSMTP_ENC_QUOTED	4 /* not really used in current version */

    #define LIBSMTP_MAX_ENC	4


    /* Charset values */

    #define LIBSMTP_CHARSET_NOCHARSET			-1
    #define LIBSMTP_CHARSET_USASCII				0
    #define LIBSMTP_CHARSET_ISO8859_1			1
    #define LIBSMTP_CHARSET_ISO8859_2			2
    #define LIBSMTP_CHARSET_ISO8859_3			3
		#define LIBSMTP_CHARSET_ISO8859_15  	4
		#define LIBSMTP_CHARSET_UTF_8 				5
    /* Need to define more here ... */

    #define LIBSMTP_MAX_CHARSET		5


  #endif /* LIB_SMTP_MIME_H */

  struct libsmtp_part_struct {
    int internal_id;	/* internal id number */
    int Type;	/* MIME type */
    GString *CustomType;	/* optional custom MIME type */
    int Subtype;	/* MIME subtype */
    GString *CustomSubtype;	/* optional custom MIME subtype */
    int Encoding;	/* MIME transfer encoding */
    int Charset;	/* optional charset for text MIME types */
    GString *Description;	/* MIME part description */
    GString *Boundary;	 /* optional Multipart boundary string */
    int Tag; /* tag for user */
    int length; // part length
  };

  struct libsmtp_part_struct *libsmtp_part_new
        (struct libsmtp_part_struct *, int, int, int, int, char *, int,
        struct libsmtp_session_struct *libsmtp_session);

  int libsmtp_mime_type_custom (char *, struct libsmtp_part_struct *);

  int libsmtp_mime_subtype_custom (char *, struct libsmtp_part_struct *);

  struct libsmtp_part_struct *libsmtp_part_query (struct libsmtp_session_struct *);

  int libsmtp_mime_headers (struct libsmtp_session_struct *);

  int libsmtp_part_send (char *, unsigned int, struct libsmtp_session_struct *);

  int libsmtp_part_next (struct libsmtp_session_struct *);

  /* internal functions */

  int libsmtp_int_check_part (struct libsmtp_part_struct *);

  const char *libsmtp_int_lookup_mime_type (struct libsmtp_part_struct *);

  const char *libsmtp_int_lookup_mime_subtype (struct libsmtp_part_struct *);

  const char *libsmtp_int_lookup_mime_charset (struct libsmtp_part_struct *);

  const char *libsmtp_int_lookup_mime_encoding (struct libsmtp_part_struct *);

  int libsmtp_int_nextpart (struct libsmtp_session_struct *);

  /* MIME related error codes >= 2048 */
  #define LIBSMTP_BADMIME	2048	/* You gave a bad type/subtype combo */
  #define LIBSMTP_NOMULTIPART	2049	/* Parent is not multipart */
  #define LIBSMTP_BADENCODING	2050	/* You gave a bad MIME/encoding combo */
  #define LIBSMTP_NOPARENT	2051	/* There is no parent */
  #define LIBSMTP_PART_EXISTS	2052	/* This part exists already */
  #define LIBSMTP_PARTSERR	2053	/* Generic parts error */
  #define LIBSMTP_PARTSFINISHED	2054	/* All parts finished */
  #define LIBSMTP_BADCHARSET	2055

  #define LIBSMTP_MAX_MIME_ERRNO	2055
/* #endif LIBSMTP_USE_MIME */
