#include "replication/backfill_receiver.hpp"

namespace replication {

void backfill_receiver_t::send(scoped_malloc<net_backfill_complete_t>& message) {
    order_token_t token = order_source->check_in();
    // debugf("recv backfill_done()\n");
    cb->backfill_done(message->time_barrier_timestamp, token);
}

void backfill_receiver_t::send(UNUSED scoped_malloc<net_backfill_delete_everything_t>& msg) {
    order_token_t token = order_source->check_in();
    // debugf("recv backfill_delete_everything()\n");
    cb->backfill_delete_everything(token);
}

void backfill_receiver_t::send(scoped_malloc<net_get_cas_t>& msg) {
    order_token_t token = order_source->check_in();
    store_key_t key(msg->key_size, msg->key);
    // debugf("recv realtime_get_cas(%.*s)\n", key.size, key.contents);
    castime_t castime(msg->proposed_cas, msg->timestamp);
    cb->realtime_get_cas(key, castime, token);
}

void backfill_receiver_t::send(stream_pair<net_sarc_t>& msg) {
    order_token_t token = order_source->check_in();
    store_key_t key(msg->key_size, msg->keyvalue);
    // debugf("recv realtime_sarc(%.*s)\n", key.size, key.contents);
    castime_t castime(msg->proposed_cas, msg->timestamp);
    cb->realtime_sarc(key, msg.stream, msg->flags, msg->exptime, castime,
                      add_policy_t(msg->add_policy), replace_policy_t(msg->replace_policy),
                      msg->old_cas, token);
}

void backfill_receiver_t::send(stream_pair<net_backfill_set_t>& msg) {
    order_token_t token = order_source->check_in();
    backfill_atom_t atom;
    atom.key.assign(msg->key_size, msg->keyvalue);
    // debugf("recv backfill_set(%.*s, t=%u, len=%ld)\n", atom.key.size, atom.key.contents, msg->timestamp.time, msg.stream->get_size());
    atom.value = msg.stream;
    atom.flags = msg->flags;
    atom.exptime = msg->exptime;
    atom.recency = msg->timestamp;
    atom.cas_or_zero = msg->cas_or_zero;
    cb->backfill_set(atom, token);
}

void backfill_receiver_t::send(scoped_malloc<net_incr_t>& msg) {
    order_token_t token = order_source->check_in();
    store_key_t key(msg->key_size, msg->key);
    // debugf("recv realtime_incr_decr(%.*s)\n", key.size, key.contents);
    castime_t castime(msg->proposed_cas, msg->timestamp);
    cb->realtime_incr_decr(incr_decr_INCR, key, msg->amount, castime, token);
}

void backfill_receiver_t::send(scoped_malloc<net_decr_t>& msg) {
    order_token_t token = order_source->check_in();
    store_key_t key(msg->key_size, msg->key);
    // debugf("recv realtime_incr_decr(%.*s)\n", key.size, key.contents);
    castime_t castime(msg->proposed_cas, msg->timestamp);
    cb->realtime_incr_decr(incr_decr_DECR, key, msg->amount, castime, token);
}

void backfill_receiver_t::send(stream_pair<net_append_t>& msg) {
    order_token_t token = order_source->check_in();
    store_key_t key(msg->key_size, msg->keyvalue);
    // debugf("recv realtime_append_prepend(%.*s)\n", key.size, key.contents);
    castime_t castime(msg->proposed_cas, msg->timestamp);
    cb->realtime_append_prepend(append_prepend_APPEND, key, msg.stream, castime, token);
}

void backfill_receiver_t::send(stream_pair<net_prepend_t>& msg) {
    order_token_t token = order_source->check_in();
    store_key_t key(msg->key_size, msg->keyvalue);
    // debugf("recv realtime_append_prepend(%.*s)\n", key.size, key.contents);
    castime_t castime(msg->proposed_cas, msg->timestamp);
    cb->realtime_append_prepend(append_prepend_PREPEND, key, msg.stream, castime, token);
}

void backfill_receiver_t::send(scoped_malloc<net_delete_t>& msg) {
    order_token_t token = order_source->check_in();
    store_key_t key(msg->key_size, msg->key);
    // debugf("recv realtime_delete(%.*s)\n", key.size, key.contents);
    cb->realtime_delete_key(key, msg->timestamp, token);
}

void backfill_receiver_t::send(scoped_malloc<net_backfill_delete_t>& msg) {
    order_token_t token = order_source->check_in();
    store_key_t key(msg->key_size, msg->key);
    // debugf("recv backfill_deletion(%.*s)\n", key.size, key.contents);
    cb->backfill_deletion(key, token);
}

void backfill_receiver_t::send(scoped_malloc<net_nop_t>& message) {
    order_token_t token = order_source->check_in();
    // debugf("recv realtime_time_barrier()\n");
    cb->realtime_time_barrier(message->timestamp, token);
}

}
