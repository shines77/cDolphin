/*
   File:          autoplay.h

   Created:       May 21, 1998

   Modified:      August 1, 2002

   Author:        Gunnar Andersson (gunnar@radagast.se)

   Contents:
*/

#ifndef __AUTOPLAY_H_
#define __AUTOPLAY_H_

#ifdef __cplusplus
extern "C" {
#endif

void
handle_event(int only_passive_events,
             int allow_delay,
             int passive_mode);

void
toggle_event_status(int allow_event_handling);

#ifdef __cplusplus
}
#endif

#endif  /* __AUTOPLAY_H_ */
