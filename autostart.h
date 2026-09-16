#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
runAutostart(const char *exedir) {
	char always[4096];
	snprintf(always, sizeof(always), "%s/autostart_always", exedir);
	if (access(always, F_OK) == 0)
		system(always);

	char flagfile[4096];
	snprintf(flagfile, sizeof(flagfile), "%s/.autostarted", exedir);

	if (access(flagfile, F_OK) == 0) {
		unlink(flagfile);
		return;
	}

	char script[4096];
	snprintf(script, sizeof(script), "%s/autostart", exedir);
	system(script);
}

void
setRestartFlag(const char *exedir) {
	char flagfile[4096];
	snprintf(flagfile, sizeof(flagfile), "%s/.autostarted", exedir);
	FILE *f = fopen(flagfile, "w");
	if (f)
		fclose(f);
}

