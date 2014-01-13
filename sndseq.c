/*
 * based on the Simple Echo pseudo-device KLD example
 * in the FreeBSD Architecture Handbook r43184 (section 9.3 - Character Devices)
 *
 * which is written by:
 * Murray Stokely
 * Soren (Xride) Straarup
 * Eitan Adler
 */

#define SNDRV_SEQ_IOCTL_PVERSION 0x40045300
#define SNDRV_SEQ_IOCTL_CLIENT_ID 0x40045301

#include <sys/types.h>
#include <sys/module.h>
#include <sys/systm.h>  /* uprintf */
#include <sys/param.h>  /* defines used in kernel.h */
#include <sys/kernel.h> /* types used in module initialization */
#include <sys/conf.h>   /* cdevsw struct */
#include <sys/uio.h>    /* uio struct */
#include <sys/malloc.h>

#define BUFFERSIZE 255

/* Function prototypes */
static d_open_t      sndseq_open;
static d_close_t     sndseq_close;
static d_read_t      sndseq_read;
static d_write_t     sndseq_write;
static d_ioctl_t     sndseq_ioctl;

/* Character device entry points */
static struct cdevsw sndseq_cdevsw = {
	.d_version = D_VERSION,
	.d_open = sndseq_open,
	.d_close = sndseq_close,
	.d_read = sndseq_read,
	.d_write = sndseq_write,
	.d_ioctl = sndseq_ioctl,
	.d_name = "sndseq",
};

/* vars */
static struct cdev *sndseq_dev;

/*
MALLOC_DECLARE(M_ECHOBUF);
MALLOC_DEFINE(M_ECHOBUF, "sndseqbuffer", "buffer for sndseq module");
*/

/*
 * This function is called by the kld[un]load(2) system calls to
 * determine what actions to take when a module is loaded or unloaded.
 */
static int
sndseq_loader(struct module *m __unused, int what, void *arg __unused)
{
	int error = 0;

	switch (what) {
	case MOD_LOAD:                /* kldload */
		error = make_dev_p(MAKEDEV_CHECKNAME | MAKEDEV_WAITOK,
		    &sndseq_dev,
		    &sndseq_cdevsw,
		    0,
		    UID_ROOT,
		    GID_WHEEL,
		    0666,
		    "snd/seq");
		if (error != 0)
			break;

		uprintf("ALSA snd/seq emulator loaded\n");
		break;
	case MOD_UNLOAD:
		destroy_dev(sndseq_dev);
		uprintf("ALSA snd/seq emulator unloaded\n");
		break;
	default:
		error = EOPNOTSUPP;
		break;
	}
	return (error);
}

static int
sndseq_open(struct cdev *dev __unused, int oflags __unused, int devtype __unused,
    struct thread *td __unused)
{
	int error = 0;

	uprintf("Opened device \"sndseq\" successfully.\n");
	return (error);
}

static int
sndseq_close(struct cdev *dev __unused, int fflag __unused, int devtype __unused,
    struct thread *td __unused)
{
	uprintf("Closing device \"sndseq\".\n");
	return (0);
}

static int
sndseq_read(struct cdev *dev __unused, struct uio *uio, int ioflag __unused)
{
	int error = 0;

	uprintf("sndseq uio read: resid=%li, offset=%li\n", uio->uio_resid, uio->uio_offset);

	return (error);
}

static int
sndseq_write(struct cdev *dev __unused, struct uio *uio, int ioflag __unused)
{
	int error = 0;

	/*
	// abusing dmesg for this is totally a good idea
	int base_resid = uio->uio_resid;
	unsigned char buf[1];
	int i;
	printf("sndseq uio write: resid=%li, offset=%li\n", uio->uio_resid, uio->uio_offset);

	printf("ddump:");
	for(i = 0; i < base_resid; i++)
	{
		uiomove(buf, 1, uio);
		printf(" %02X", buf[0]);
	}
	printf("\n");
	*/

	uio->uio_offset += uio->uio_resid;
	uio->uio_resid = 0;

	if (error != 0)
		uprintf("Write failed: bad address!\n");

	return (error);
}

//
static int
sndseq_ioctl(struct cdev *dev, u_long cmd, caddr_t data, int fflag, struct thread *td)
{
	int error = 0;

	uprintf("sndseq ioctl: cmd=%016lx/%li data=%p fflag=%i\n",
		cmd, cmd, data, fflag);
	switch(cmd)
	{
		case SNDRV_SEQ_IOCTL_PVERSION:
			data[3] = 0;
			data[2] = 1;
			data[1] = 0;
			data[0] = 1;
			break;

		case SNDRV_SEQ_IOCTL_CLIENT_ID:
			data[3] = 0;
			data[2] = 0;
			data[1] = 0;
			data[0] = 0x21;
			break;
		
		case 0xC0BC5310:
			// set client name(?)
			uprintf("sndseq ioctl: databyte=%02X\n", data[0]);
			break;

		case 0x80BC5311:
			// set client name(?)
			uprintf("sndseq ioctl: databyte=%02X\n", data[0]);
			break;

		case 0xC0A85320:
			// create port
			uprintf("sndseq ioctl: databyte=%02X\n", data[0]);
			break;

		case 0xC08C5332:
			// create queue
			uprintf("sndseq ioctl: databyte=%02X\n", data[0]);
			break;

		case 0x80505330:
			// open port
			uprintf("sndseq ioctl: databyte=%02X\n", data[0]);
			break;

		case 0x802C5342:
			// set tempo
			// (why is this empty?!)
			uprintf("sndseq ioctl: databyte=%02X%02X%02X%02X\n"
				, data[3]
				, data[2]
				, data[1]
				, data[0]);
			break;

		case 0xC058534B:
			// sync output 1
			uprintf("sndseq ioctl: databyte=%02X\n", data[0]);
			break;

		case 0x8058534C:
			// sync output 2
			uprintf("sndseq ioctl: databyte=%02X\n", data[0]);
			break;

		default:
			error = EINVAL;
			break;
	}

	return (error);
}

DEV_MODULE(sndseq, sndseq_loader, NULL);

