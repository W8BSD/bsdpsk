LOCALBASE?=	/usr/local
DESTDIR?=	${LOCALBASE}
BINDIR?=	/bin
MANDIR?=	/man/man

PROG=	bsdpsk
LDADD=	-lm
SRCS=	bsdpsk.c audio.c psk_demod.c varicode.c filters.c
DPADD=	$(LIBM}

.include <bsd.prog.mk>
