#include "btree/delete.hpp"
#include "btree/modify_oper.hpp"

#include "btree/delete_queue.hpp"

struct btree_delete_oper_t : public btree_modify_oper_t<memcached_value_t> {

    btree_delete_oper_t(bool dont_record, btree_slice_t *_slice) : dont_put_in_delete_queue(dont_record), slice(_slice) { }

    delete_result_t result;
    bool dont_put_in_delete_queue;
    btree_slice_t *slice;

    bool operate(transaction_t *txn, scoped_malloc<memcached_value_t>& value) {
        if (value) {
            result = dr_deleted;
            {
                blob_t b(value->value_ref(), blob::btree_maxreflen);
                b.unappend_region(txn, b.valuesize());
            }
            value.reset();
            return true;
        } else {
            result = dr_not_found;
            return false;
        }
    }

    int compute_expected_change_count(UNUSED block_size_t block_size) {
        return 1;
    }

    void do_superblock_sidequest(transaction_t *txn,
                                 superblock_t *superblock,
                                 repli_timestamp_t recency,
                                 const store_key_t *key) {

        if (!dont_put_in_delete_queue) {
            slice->assert_thread();

            block_id_t delete_queue_root = superblock->get_delete_queue_block();
            if (delete_queue_root != NULL_BLOCK_ID) {
                replication::add_key_to_delete_queue(slice->delete_queue_limit(), txn, delete_queue_root, recency, key);
            } else {
                crash("Failed to locate delete queue\n"); // TODO: Is it ok to fail in this case?
            }
        }
    }
};

delete_result_t btree_delete(value_sizer_t<memcached_value_t> *sizer, const store_key_t &key, bool dont_put_in_delete_queue, btree_slice_t *slice, repli_timestamp_t timestamp, order_token_t token) {
    btree_delete_oper_t oper(dont_put_in_delete_queue, slice);
    run_btree_modify_oper<memcached_value_t>(sizer, &oper, slice, key, castime_t(BTREE_MODIFY_OPER_DUMMY_PROPOSED_CAS, timestamp), token);
    return oper.result;
}
