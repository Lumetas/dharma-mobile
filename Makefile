# dharma - dynamic window manager
# See LICENSE file for copyright and license details.

include config.mk

SRC = drw.c dharma.c util.c events.c
OBJ = ${SRC:.c=.o}
MAKEFILE_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

SRC_L = dharma-launcher.c launcher_icons.c
OBJ_L = ${SRC_L:.c=.o}
BIN_L = dharma-launcher

all: options dharma ${BIN_L}

${BIN_L}: ${OBJ_L}
	${CC} -o $@ ${OBJ_L} ${LDFLAGS} ${LIBS_L}

options:
	@echo dharma build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	${CC} -c ${CFLAGS} $<

${OBJ}: config.h config.mk

${OBJ_L}: launcher_icons.h config.mk

config.h:
	cp config.def.h $@

dharma: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f dharma ${OBJ} ${BIN_L} ${OBJ_L} dharma-${VERSION}.tar.gz

dist: clean
	mkdir -p dharma-${VERSION}
	cp -R LICENSE Makefile README config.def.h config.mk\
		dharma.1 drw.h util.h ${SRC} dharma.png transient.c dharma-${VERSION}
	tar -cf dharma-${VERSION}.tar dharma-${VERSION}
	gzip dharma-${VERSION}.tar
	rm -rf dharma-${VERSION}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	ln -sf ${MAKEFILE_DIR}/dharma ${DESTDIR}${PREFIX}/bin/dharma
	cp -f dharmactl ${DESTDIR}${PREFIX}/bin
	cp -f dharma.desktop /usr/share/xsessions/dharma.desktop
	chmod 755 ${DESTDIR}${PREFIX}/bin/dharma
	chmod +x ${DESTDIR}${PREFIX}/bin/dharmactl
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	sed "s/VERSION/${VERSION}/g" < dharma.1 > ${DESTDIR}${MANPREFIX}/man1/dharma.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/dharma.1
	ln -sf ${MAKEFILE_DIR}/${BIN_L} ${DESTDIR}${PREFIX}/bin/${BIN_L}
	chmod 755 ${DESTDIR}${PREFIX}/bin/${BIN_L}

init:
	mkdir -p ~/.config/dharma
	ln -sf ${MAKEFILE_DIR}/config.h ~/.config/dharma/config.h
	ln -sf ${MAKEFILE_DIR}/autostart ~/.config/dharma/autostart
	ln -sf ${MAKEFILE_DIR}/autostart_always ~/.config/dharma/autostart_always

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/dharma\
		${DESTDIR}${MANPREFIX}/man1/dharma.1
	rm -f ${DESTDIR}${PREFIX}/bin/${BIN_L}

.PHONY: all options clean dist install uninstall


DAEMON_SRC  = daemon/wake-daemon.sh
DAEMON_BIN  = ${DESTDIR}${PREFIX}/bin/dharma-wake-daemon
DAEMON_UNIT = ${DESTDIR}${PREFIX}/lib/systemd/user/dharma-wake-daemon.service
DAEMON_CONF = ${DESTDIR}/etc/dharma/wake-daemon.conf

reload_daemon:
	-systemctl --user daemon-reload
	-systemctl --user reset-failed dharma-wake-daemon 2>/dev/null

install_daemon:
	mkdir -p ${DESTDIR}${PREFIX}/bin
	install -m755 ${MAKEFILE_DIR}/${DAEMON_SRC} ${DAEMON_BIN}
	mkdir -p ${DESTDIR}${PREFIX}/lib/systemd/user
	install -m644 ${MAKEFILE_DIR}/daemon/wake-daemon.service ${DAEMON_UNIT}
	mkdir -p ${DESTDIR}/etc/dharma
	@[ -f ${DAEMON_CONF} ] || install -m644 ${MAKEFILE_DIR}/daemon/wake-daemon.conf ${DAEMON_CONF}
	@echo "installed."
	@echo "  binary: ${DAEMON_BIN}"
	@echo "  unit:   ${DAEMON_UNIT}"
	@echo "  config: ${DAEMON_CONF}"
	@echo "restart Dharma to start the daemon."

uninstall_daemon:
	rm -f ${DAEMON_BIN} ${DAEMON_UNIT} ${DAEMON_CONF}
	-rmdir ${DESTDIR}/etc/dharma 2>/dev/null
	@echo "uninstalled."
	@echo "restart Dharma to stop the daemon."

.PHONY: install_daemon uninstall_daemon
