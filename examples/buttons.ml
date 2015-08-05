(*
 * buttons.ml
 * ----------
 * Copyright : (c) 2011, Jeremie Dimino <jeremie@dimino.org>
 * Licence   : BSD3
 *
 * This file is a part of Lambda-Term.
 *)

open Lwt
open LTerm_widget

let main () =
  let waiter, wakener = wait () in

  let vbox = new vbox in
  let button = new button "exit" in
  button#on_click (wakeup wakener);
  vbox#add button;

  for i = 0 to 2 do
    let hbox = new hbox in
    hbox#add (new button ("button" ^ string_of_int (i * 3 + 1)));
    hbox#add ~expand:false (new vline);
    hbox#add (new button ("button" ^ string_of_int (i * 3 + 2)));
    hbox#add ~expand:false (new vline);
    hbox#add (new button ("button" ^ string_of_int (i * 3 + 3)));
    vbox#add ~expand:false (new hline);
    vbox#add hbox
  done;

  let frame = new frame in
  frame#set vbox;

  Lazy.force LTerm.stdout
  >>= fun term ->
  run term frame waiter

let () = Lwt_main.run (main ())
