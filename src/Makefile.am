## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = zakaz

zakaz_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	sqlite3.c SQLiteConnect.c \
	callbacks.c callbacks.h \
	app.c \	
	chart.c \	
	chartsheet.c \	
	comment.c \	
	content_types.c \	
	core.c \	
	custom.c \	
	drawing.c \	
	format.c \	
	hash_table.c \	
	metadata.c \	
	packager.c \	
	relationships.c \	
	shared_strings.c \	
	styles.c \	
	table.c \	
	theme.c \	
	utility.c \	
	vml.c \	
	workbook.c \	
	worksheet.c \	
	xmlwriter.c	\
	emyg_dtoa.c \
	md5.c \
	ioapi.c \	
	iowin32.c \	
	mztools.c \	
	zip.c \
	unzip.c \
	tmpfileplus.c

zakaz_LDADD = @PACKAGE_LIBS@ $(INTLLIBS) -lz

