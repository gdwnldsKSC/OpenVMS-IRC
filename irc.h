// General housekeeping header

// VAX (and pre 7.3-2 Alpha) don't have snprintf, so we bring one with us
// in snprintf_vms.c - right now we only handle VAX breakage
#ifdef __VAX
#define snprintf snprintf_vms
int snprintf_vms(char *str, size_t len, const char *fmt, ...);
#endif
