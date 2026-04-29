/**
 * @brief this subsystem enables chaining different sloted structures
 * 
 * @note the current iteration of the subsystem supports only the singly-linking
 * 
 * Author: Omid Kandelusy
 */
// ====================================================================================
#ifndef SLOT_LINKER_HEADER_GUARD
#define SLOT_LINKER_HEADER_GUARD

/** including the required header files */

/** standard c++ header files */
#include <cstdint>
#include <cstddef>


// ====================================================================================
// typedefs and macros

/** a generic linking node type */
typedef struct node_s {
    bool in_use;
    struct node_s * next;
    void * user_data;
} sl_node_t;


/** a generic anchoring point type */
typedef struct {
    sl_node_t *base;
} sl_anchor_t;


class Slot_linker{
    private:
    sl_anchor_t anchor;

    public:
    Slot_linker(const sl_anchor_t& a) : anchor(a) {};

    sl_node_t* available_spot() const {
        sl_node_t *current = anchor.base;
        while(current != nullptr){
            if (!current->in_use){
                return current;
            }
            current = current->next;
        }

        return NULL;
    }

    int active_length() const {
        sl_node_t * current = anchor.base;
        int counter = 0;
        while (current != nullptr){
            if (current->in_use){
                counter++;
            }
            current = current->next;
        }

        return counter;
    }
};


#endif