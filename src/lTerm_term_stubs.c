/*
 * lTerm_term_stubs.c
 * ------------------
 * Copyright : (c) 2011, Jeremie Dimino <jeremie@dimino.org>
 * Licence   : BSD3
 *
 * This file is a part of Lambda-Term.
 */

#include <caml/alloc.h>
#include <caml/fail.h>
#include <caml/memory.h>
#include <caml/signals.h>
#include <caml/unixsupport.h>

#if defined(_WIN32) || defined(_WIN64)

/* +-----------------------------------------------------------------+
   | Terminal sizes on Windows                                       |
   +-----------------------------------------------------------------+ */

#include <windows.h>
#include <wincon.h>

CAMLprim value lt_term_get_size_from_fd(value fd)
{
  CONSOLE_SCREEN_BUFFER_INFO info;
  value result;

  if (!GetConsoleScreenBufferInfo(Handle_val(fd), &info)) {
    win32_maperr(GetLastError());
    uerror("GetConsoleScreenBufferInfo", Nothing);
  }

  result = caml_alloc_tuple(2);
  Field(result, 0) = Val_int(info.srWindow.Bottom - info.srWindow.Top + 1);
  Field(result, 1) = Val_int(info.srWindow.Right - info.srWindow.Left + 1);
  return result;
}
/*
CAMLprim value lt_term_set_size_from_fd(value fd, value val_size)
{
  CONSOLE_SCREEN_BUFFER_INFO info;
  SMALL_RECT rect;

  if (!GetConsoleScreenBufferInfo(Handle_val(fd), &info)) {
    win32_maperr(GetLastError());
    uerror("GetConsoleScreenBufferInfo", Nothing);
  }

  rect;
  rect.Top = info.srWindow.Top;
  rect.Left = info.srWindow.Left;
  rect.Bottom = rect.Top + Int_val(Field(val_size, 0)) - 1;
  rect.Right = rect.Left + Int_val(Field(val_size, 1)) - 1;

  if (!SetConsoleWindowInfo(Handle_val(fd), TRUE, &rect)) {
    win32_maperr(GetLastError());
    uerror("SetConsoleWindowInfo", Nothing);
  }

  return Val_unit;
}
*/
#else

/* +-----------------------------------------------------------------+
   | Terminal sizes on Unix                                          |
   +-----------------------------------------------------------------+ */

#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>

CAMLprim value lt_term_get_size_from_fd(value fd)
{
  struct winsize size;

  if (ioctl(Int_val(fd), TIOCGWINSZ, &size) < 0)
    uerror("ioctl", Nothing);

  value result = caml_alloc_tuple(2);
  Field(result, 0) = Val_int(size.ws_row);
  Field(result, 1) = Val_int(size.ws_col);
  return result;
}

/*
CAMLprim value lt_term_set_size_from_fd(value fd, value val_size)
{
  struct winsize size;

  if (ioctl(Int_val(fd), TIOCGWINSZ, &size) < 0)
    uerror("ioctl", Nothing);

  int row = Int_val(Field(val_size, 0));
  int col = Int_val(Field(val_size, 1));

  size.ws_xpixel = size.ws_xpixel * col / size.ws_col;
  size.ws_ypixel = size.ws_ypixel * row / size.ws_row;
  size.ws_row = row;
  size.ws_col = col;

  if (ioctl(Int_val(fd), TIOCSWINSZ, &size) < 0)
    uerror("ioctl", Nothing);

  return Val_unit;
}
*/

/* +-----------------------------------------------------------------+
   | Signals management                                              |
   +-----------------------------------------------------------------+ */

#include <pthread.h>
#include <signal.h>
#include <assert.h>

static pthread_t signal_manager_thread = 0;

CAMLprim value lt_term_init_signal_manager_thread()
{
  signal_manager_thread = pthread_self();
  return Val_unit;
}

static void handle_signal(int signum)
{
  assert(pthread_self() != signal_manager_thread);
  pthread_kill(signal_manager_thread, signum);
}

CAMLextern int caml_convert_signal_number (int);
CAMLextern int caml_rev_convert_signal_number (int);

CAMLprim value lt_term_set_signal(value val_signum)
{
  struct sigaction sa;
  int signum = caml_convert_signal_number(Int_val(val_signum));

  if (signum < 0 || signum >= NSIG)
    caml_invalid_argument("LTerm.Signals.set");

  sa.sa_handler = handle_signal;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  if (sigaction(signum, &sa, NULL) == -1)
    uerror("sigaction", Nothing);
  return Val_unit;
}

CAMLprim value lt_term_ocaml_signal_of_signo(value val_signo)
{
  return (Val_int(caml_rev_convert_signal_number(Int_val(val_signo))));
}

#endif
