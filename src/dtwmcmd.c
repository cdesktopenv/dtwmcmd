/*
 * Send f. commands to dtwm.  I have no idea whether or not the use of the
 * _DT_WM_REQUEST property in this fashion is intended to be an exposed
 * interface.
 *
 * Compiled on Tru64 Unix 4.0, HP-UX 11.00, and Solaris 7 with the command:
 *    cc dtwmcmd.c -o dtwmcmd -lX11
 * (On Solaris, that may assume SunPRO C or that "cc" as a link to gcc will
 * be found via $PATH before /usr/ucb/cc)
 *
 * Compiled on Solaris 2.5 or later with the command:
 *    gcc -I/usr/dt/include -I/usr/openwin/include -L/usr/openwin/lib \ 
 *       -R/usr/openwin/lib -lX11 -lsocket -lnsl dtwmcmd.c -o dtwmcmd
 *
 * rlhamil@mindwarp.smart.net 8/11/1998
 * updated 10/31/1998 because quoting a command with args got tiresome
 *         (although one still can for compatibility)
 *         The only change is in main() where it malloc()'s enough space
 *         to hold the command and any args, plus spaces and a null byte,
 *         and then copies that whole mess into the malloc()'ed buffer.
 *
 * updated 12/30/1998 to fix a couple of memory leaks in the dtwmcmd()
 *         function, which could have proved troublesome if one wanted
 *         to call the function repeatedly in some other code.
 *
 * winterz@clark.net 02/04/2000
 * updated to compile on Tru64 Unix 4.0 and HP-UX 11.00
 *
 */

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <Xm/MwmUtil.h>
#include <string.h>

/*
 * The function that does the real work - separated out from main() in
 * case  you want to use it in some other C code.
 */
int dtwmcmd(Display *display,char *command)
{
   Atom _DT_WM_REQUEST,_MOTIF_WM_INFO,_DT_WORKSPACE_CURRENT;
   Atom actual_type_return;
   Window wm_window;
   int actual_format_return;
   unsigned long nitems_return;
   unsigned long bytes_after_return;
   unsigned char *prop_return;

   /* Mwm and dtwm both put a "_MOTIF_WM_INFO" property on the root window,
    * whose value is a structure consisting of a long containing window
    * manager state flags, and a window id of the private, input-only
    * window manager window where the window manager keeps the properties
    * that let it restore its state after a restart.  Dtwm further arranges
    * to be notified whenever the "_DT_WM_REQUEST" property is updated on
    * its private window, and if the string value looks like a window manager
    * command, it executes it.
    */

   if ((_MOTIF_WM_INFO=XInternAtom(display,_XA_MOTIF_WM_INFO,True))==None ||
   XGetWindowProperty(display,DefaultRootWindow(display),_MOTIF_WM_INFO,
   0L,PROP_MOTIF_WM_INFO_ELEMENTS,False,_MOTIF_WM_INFO,
   &actual_type_return,&actual_format_return,&nitems_return,
   &bytes_after_return,&prop_return) != Success)
      return BadRequest;

   if (actual_type_return != _MOTIF_WM_INFO || actual_format_return!=32 ||
         nitems_return < 2) {
      XFree(prop_return);
      return BadRequest;
   }

   wm_window = ((MotifWmInfo *) prop_return) -> wm_window;
   XFree(prop_return); /* if we free it now, we don't have to remember later */

   /* We should now have the window id of the private window manager window.
    * Since mwm demonstrably doesn't respond to what we're going to do, we
    * want a way to distinguish mwm and dtwm, so we can return an error
    * indication if dtwm is not what's running.  Checking for a property
    * unique to dtwm seems to be an easy way to do this.
    */

   if ((_DT_WORKSPACE_CURRENT=
   XInternAtom(display,"_DT_WORKSPACE_CURRENT",True))==None ||
   XGetWindowProperty(display,wm_window,_DT_WORKSPACE_CURRENT,
   0L,1,False,XA_ATOM,
   &actual_type_return,&actual_format_return,&nitems_return,
   &bytes_after_return,&prop_return) != Success)
      return BadRequest;

   if (actual_type_return != XA_ATOM) {
      XFree(prop_return);
      return BadRequest;
   }

   XFree(prop_return);

   /* Now we just get the Atom for "_DT_WM_REQUEST", and put the property
    * out there with the value from our argument.
    */
   if ((_DT_WM_REQUEST=XInternAtom(display,"_DT_WM_REQUEST",False))!=None)
      XChangeProperty(display,wm_window,_DT_WM_REQUEST,XA_STRING,8,
      PropModeReplace,(unsigned char *) command,strlen(command));
   else {
      return BadAlloc;
   }
   XFlush(display);
   return Success;
}

#include <stdio.h>
#include <stdlib.h>

void usage(char *cmd)
{
   fprintf(stderr,"usage: %s [-display displayname] dtwm_command\n",cmd);
   exit(2);
}

int main(int argc, char **argv)
{
   int rc, dftdpy=True;
   Display *display;
   char *command;
   size_t totlen=0;
   int x;

   if (argc<2)
      usage(argv[0]);
   if (!(dftdpy=strcmp(argv[1],"-display")) && argc<4)
      usage(argv[0]);

   if ((display=XOpenDisplay(dftdpy?NULL:argv[2])) == NULL) {
      fprintf(stderr,"%s: can't open display\n",argv[0]);
      return 1;
   }

   for (x=(dftdpy?1:3);x<argc;x++)
      totlen+=(strlen(argv[x])+1);
   if ((command=(char *) malloc(totlen))==NULL) {
      fprintf(stderr,
         "%s: insufficient memory to concatenate command and args\n",argv[0]);
      return 127;
   }
   for (x=(dftdpy?1:3),*command='\0';x<argc;x++) {
      strcat(command,argv[x]);
      if (x<argc-1)
         strcat(command," ");
   }
   switch(rc=dtwmcmd(display,command)) {
   case Success:
      break;
   case BadRequest:
      fprintf(stderr,"%s: dtwm doesn't appear to be running\n",argv[0]);
      break;
   case BadAlloc:
      fprintf(stderr,"%s: failed, server may be out of memory\n",argv[0]);
      break;
   default:
      fprintf(stderr,"%s: failed, unanticipated error %d\n",argv[0],rc);
      break;
   }

   XCloseDisplay(display);
   return rc;
}
