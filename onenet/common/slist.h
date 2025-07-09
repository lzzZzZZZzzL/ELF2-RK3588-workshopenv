/**
 * Copyright (c), 2012~2018 iot.10086.cn All Rights Reserved
 * @file        slist.h
 * @brief       Single direction linked list
 */

#ifndef __SLIST_H__
#define __SLIST_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"
#include "plat_osl.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* External Definition（Constant and Macro )                                 */
/*****************************************************************************/

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/
/**
 * @brief node definition
 *
 */
struct slist_node
{
    /** Point to the next data node，When the current node is the last node, it is NULL*/
    struct slist_node *next;
};

/**
 * @brief header definition.
 *
 */
struct slist_head
{
    /** Points to the first node of the linked list，When the linked list is empty，it is NULL*/
    struct slist_node *head;
    /** Points to the last node of the linked list，When the linked list is empty, it is NULL*/
    struct slist_node *tail;
    /** Number of nodes in the current linked list*/
    uint32_t           cnt;
};
/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
/**
 * Initialize a slist
 * @param head Header structure of unidirectional linked list
 * @retval  0 - Succeed
 * @retval -1 - Failed
 */
int32_t slist_init(struct slist_head *head);

/**
 * Query the number of nodes in the specified slist
 * @param head Header structure of unidirectional linked list
 * @retval  -1 - Error
 * @retval >=0 - Number of nodes
 */
uint32_t slist_get_cnt(struct slist_head *head);

/**
 * Insert a new node after the specified node
 * @param head slist header
 * @param current Specify where nodes need to be inserted
 * @param node Specify the new nodes to be inserted
 * @retval  0 - Insertion successful
 * @retval -1 - Insertion failed
 */
int32_t slist_insert_after(struct slist_head *head, struct slist_node *current, struct slist_node *node);

/**
 * Insert a new node in the linked list header
 * @param head slist header
 * @param node New nodes that need to be inserted
 * @retval  0 - Insertion successful
 * @retval -1 - Insertion failed
 */
int32_t slist_insert_head(struct slist_head *head, struct slist_node *node);

/**
 * Insert a new node at the end of slist
 * @param head slist header
 * @param node New nodes that need to be inserted
 * @retval  0 - Insertion successful
 * @retval -1 - Insertion failed
 */
int32_t slist_insert_tail(struct slist_head *head, struct slist_node *node);

/**
 * Delete Header Node
 * @param head slist header
 * @retval  0 - Delete successfully
 * @retval -1 - Delete failed
 */
int32_t slist_remove_head(struct slist_head *head);

/**
 * Get the next node of the current node
 * @param current Specify the current node
 * @return Non-NULL - Next node pointer
 */
struct slist_node *slist_get_next(struct slist_node *current);

/**
 * Get the head node of a slist
 * @param head slist header
 * @return Non-NULL - Head node pointer
 */
struct slist_node *slist_get_head(struct slist_head *head);

/**
 * Get the tail node of a slist
 * @param head slist header
 * @return Non-NULL - Tail node pointer
 */
struct slist_node *slist_get_tail(struct slist_head *head);

#ifdef __cplusplus
}
#endif

#endif
