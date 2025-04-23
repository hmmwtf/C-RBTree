#ifndef _RBTREE_H_
#define _RBTREE_H_

#include <stddef.h>

// 색상 정보를 나타내는 열거형
// RBTREE_RED: 레드 노드
// RBTREE_BLACK: 블랙 노드
typedef enum { RBTREE_RED, RBTREE_BLACK } color_t;

// 키 타입 정의 (정수형)
typedef int key_t;

// 트리의 노드를 정의하는 구조체
struct node_t {
  color_t color;        // 노드 색상
  key_t key;            // 노드에 저장된 키 값
  struct node_t *parent; // 부모 노드 포인터
  struct node_t *left;   // 왼쪽 자식 포인터
  struct node_t *right;  // 오른쪽 자식 포인터
};
typedef struct node_t node_t;

// 레드-블랙 트리 전체를 나타내는 구조체
typedef struct {
  node_t *root; // 트리의 루트 노드
  node_t *nil;  // NIL 노드를 가리키는 센티넬 포인터 (모든 빈 자식은 이 노드를 가리킴)
} rbtree;

// 새로운 레드-블랙 트리를 생성하고 초기화하여 반환
// 메모리 할당 후 루트와 nil 센티넬을 설정
rbtree *new_rbtree(void);

// 레드-블랙 트리의 모든 노드를 해제하고
// 트리 구조체도 함께 해제
void delete_rbtree(rbtree *);

// 키 값을 트리에 삽입하고, 삽입된 노드 포인터를 반환
// 삽입 후 레드-블랙 트리 속성을 유지하도록 재조정
node_t *rbtree_insert(rbtree *, const key_t);

// 특정 키 값을 가진 노드를 검색하여 반환
// 찾지 못하면 nil 포인터를 반환
node_t *rbtree_find(const rbtree *, const key_t);

// 트리 내에서 가장 작은 키를 가진 노드를 반환
// 트리가 비어 있으면 nil을 반환
node_t *rbtree_min(const rbtree *);

// 트리 내에서 가장 큰 키를 가진 노드를 반환
// 트리가 비어 있으면 nil을 반환
node_t *rbtree_max(const rbtree *);

// 특정 노드를 트리에서 삭제
// 삭제 후 레드-블랙 트리 속성을 유지하도록 재조정
// 성공 시 0, 실패 시 음수 반환
int rbtree_erase(rbtree *, node_t *);

// 레드-블랙 트리에 저장된 키들을 오름차순으로 배열에 복사
// 반환 값은 복사된 요소의 개수
int rbtree_to_array(const rbtree *, key_t *, const size_t);

// 서브트리의 모든 노드를 재귀적으로 해제
void free_subtree(rbtree *t, node_t *n);

// 중위 순회로 키 값을 배열에 복사
void inorder_search(const rbtree *t, key_t *arr, size_t size, int *idx, node_t *n);

// 삽입 후 레드–블랙 트리 속성(색상·균형) 복원
void rb_insert_fixup(rbtree *t, node_t *z);

// x를 기준으로 서브트리를 좌회전
void rb_left_rotation(rbtree *t, node_t *x);

// x를 기준으로 서브트리를 우회전
void rb_right_rotation(rbtree *t, node_t *x);

// 트리에서 old 노드를 new 노드로 교체 (부모 포인터만 수정)
void rb_node_change(rbtree *, node_t *, node_t *);

// 주어진 서브트리에서 가장 작은 키를 가진 노드 반환
node_t *rb_tree_min_subtree(rbtree *, node_t *);

void rb_delete_fixup(rbtree *, node_t *);

#endif  // _RBTREE_H_;
