#ifndef LIBTORRENT_TRACKER_CONTROLLER_H
#define LIBTORRENT_TRACKER_CONTROLLER_H

#import <functional>
#import <string>

#import <torrent/common.h>
#import <torrent/tracker.h>

// Refactor:
namespace rak { class priority_item; }

namespace torrent {

class AddressList;
class TrackerList;
struct tracker_controller_private;

class LIBTORRENT_EXPORT TrackerController {
public:
  typedef AddressList address_list;

  static const int flag_send_update      = 0x1;
  static const int flag_send_completed   = 0x2;
  static const int flag_send_start       = 0x4;
  static const int flag_send_stop        = 0x8;

  static const int flag_active           = 0x10;
  static const int flag_requesting       = 0x20;
  static const int flag_failure_mode     = 0x40;
  static const int flag_promiscuous_mode = 0x80;

  static const int mask_send = flag_send_update | flag_send_start | flag_send_stop | flag_send_completed;

  static const int enable_dont_reset_stats = 0x1;

  static const int close_disown_stop       = 0x1 << Tracker::EVENT_STOPPED;
  static const int close_disown_completed  = 0x1 << Tracker::EVENT_COMPLETED;

  TrackerController(TrackerList* trackers);
  ~TrackerController();

  int                 flags() const               { return m_flags; }

  bool                is_active() const           { return m_flags & flag_active; }
  bool                is_requesting() const       { return m_flags & flag_requesting; }
  bool                is_failure_mode() const     { return m_flags & flag_failure_mode; }
  bool                is_promiscuous_mode() const { return m_flags & flag_promiscuous_mode; }

  TrackerList*        tracker_list()        { return m_tracker_list; }
  TrackerList*        tracker_list() const  { return m_tracker_list; }

  int64_t             next_timeout() const;
  int64_t             next_scrape() const;
  uint32_t            seconds_to_next_timeout() const;
  uint32_t            seconds_to_next_scrape() const;

  void                manual_request(bool request_now);
  void                scrape_request(uint32_t seconds_to_request);

  //protected:
  void                send_start_event();
  void                send_stop_event();
  void                send_completed_event();
  void                send_update_event();

  void                close(int flags = close_disown_stop | close_disown_completed);

  void                enable(int enable_flags = 0);
  void                disable();

  void                start_requesting();
  void                stop_requesting();

  uint32_t            receive_success(Tracker* tb, address_list* l);
  void                receive_failure(Tracker* tb, const std::string& msg);
  void                receive_scrape(Tracker* tb);

  void                receive_tracker_enabled(Tracker* tb);
  void                receive_tracker_disabled(Tracker* tb);

  // TEMP:
  rak::priority_item* task_timeout();
  rak::priority_item* task_scrape();

private:
  TrackerController() = delete;
  void operator = (const TrackerController&) = delete;

  void                do_timeout();
  void                do_scrape();

  void                update_timeout(uint32_t seconds_to_next);

  inline int          current_send_state() const;

  int                 m_flags;
  TrackerList*        m_tracker_list;

  // Refactor this out.
  tracker_controller_private* m_private;

public:
  auto& slot_timeout()          { return m_slot_timeout; }
  auto& slot_success()          { return m_slot_success; }
  auto& slot_failure()          { return m_slot_failure; }
  auto& slot_tracker_enabled()  { return m_slot_tracker_enabled; }
  auto& slot_tracker_disabled() { return m_slot_tracker_disabled; }

private:
  std::function<void (void)>               m_slot_timeout;
  std::function<uint32_t (AddressList*)>   m_slot_success;
  std::function<void (const std::string&)> m_slot_failure;
  std::function<void (Tracker*)>           m_slot_tracker_enabled;
  std::function<void (Tracker*)>           m_slot_tracker_disabled;
};

uint32_t tracker_next_timeout(Tracker* tracker, int controller_flags);
uint32_t tracker_next_timeout_update(Tracker* tracker);
uint32_t tracker_next_timeout_promiscuous(Tracker* tracker);

}

#endif
