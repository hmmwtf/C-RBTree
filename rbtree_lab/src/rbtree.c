/*
 * rbtree_erase: 주어진 노드 p를 트리에서 삭제하고, 필요한 경우 색상·구조를 복구
 * - t: 삭제 대상이 포함된 R-B 트리
 * - p: 삭제할 노드
 * 반환: 항상 0 (성공)
*/
#include "rbtree.h"
#include <stdlib.h>

rbtree *new_rbtree(void) {
  // rbtree 구조체를 위한 메모리를 동적 할당
  rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));
  if (p == NULL) {
    // 메모리 할당 실패 시 NULL 반환
    return NULL;
  }

  // nil 노드(Sentinel node)를 위한 메모리 동적 할당
  node_t *nil = (node_t *)calloc(1, sizeof(node_t));
  if (nil == NULL){
    free(nil);  // calloc 실패지만, 안전하게 nil 해제 시도 (불필요하긴 함)
    return NULL;
  }

  // nil 노드는 항상 BLACK이며, 자기 자신을 가리킴
  nil->color = RBTREE_BLACK;  // nil 노드는 항상 검정색
  nil->key = 0;               // 임의 값, 의미 없음
  nil->parent = nil;          // 부모를 자기 자신으로 설정
  nil->left = nil;            // 왼쪽 자식도 자기 자신
  nil->right = nil;           // 오른쪽 자식도 자기 자신

  // 트리 초기화
  p->nil = nil;   // nil 노드를 트리에 연결
  p->root = nil;  // root도 초기에는 nil을 가리킴

  return p;       // 초기화된 트리 반환
}

void free_subtree(rbtree *t, node_t *n) {
  // nil 노드는 해제하지 않음 (종료 조건)
  if (n == t->nil){
    return;
  }

  // 왼쪽 서브트리 먼저 해제
  free_subtree(t, n->left);
  // 오른쪽 서브트리 해제
  free_subtree(t, n->right);
  // 현재 노드 해제
  free(n);
}

void delete_rbtree(rbtree *t) {
  // 트리가 NULL이면 아무 작업도 하지 않음
  if (t == NULL){
    return;
  }

  // 루트 노드부터 시작하여 모든 노드 재귀적으로 해제
  free_subtree(t, t->root);
  // nil 노드도 따로 해제
  free(t->nil);
  // 트리 구조체 자체 해제
  free(t);
}

node_t *rbtree_insert(rbtree *t, const key_t key) {
  // 1) 트리가 NULL이면 삽입 불가능하므로 NULL 반환
  if (t == NULL) {
    return NULL;
  }

  // 2) 새 노드를 위한 메모리 할당
  node_t *new_node = (node_t *)calloc(1, sizeof(node_t));
  if (new_node == NULL) {
    // 메모리 할당 실패 시 NULL 반환
    return NULL;
  }

  // 3) 새 노드 초기화
  new_node->key    = key;         // 키값 설정
  new_node->color  = RBTREE_RED;  // 새 노드는 RED로 시작
  new_node->left   = t->nil;      // 좌우 자식을 nil로 연결
  new_node->right  = t->nil;
  new_node->parent = t->nil;      // 부모도 일단 nil로 초기화

  // 4) 트리에서 삽입 위치 탐색
  node_t *cur = t->root;          // 루트부터 시작
  while (cur != t->nil) {
    if (key < cur->key) {
      // 삽입할 키가 현재 노드 키보다 작으면 왼쪽 서브트리 탐색
      if (cur->left == t->nil) {
        cur->left = new_node;     // 빈 자리에 새 노드 연결 후 반복문 탈출
        break;
      }
      cur = cur->left;
    } else {
      // 삽입할 키가 현재 노드 키보다 크거나 같으면 오른쪽 서브트리 탐색
      if (cur->right == t->nil) {
        cur->right = new_node;    // 빈 자리에 새 노드 연결 후 반복문 탈출
        break;
      }
      cur = cur->right;
    }
  }

  // 5) 새 노드의 부모 설정
  new_node->parent = cur;

  // 6) 트리가 비어 있었던 경우(루트가 nil이었을 때)
  if (cur == t->nil) {
    t->root = new_node;         // 새 노드를 루트로 설정
  }

  // 7) R-B 트리 성질 위반 시 복구 함수 호출
  rb_insert_fixup(t, new_node);

  // 8) 최종적으로 루트 노드를 반환
  return t->root;
}

void rb_insert_fixup(rbtree *t, node_t *z) {
  // 1) 삽입된 노드 z 의 부모가 RED인 동안 반복 => 붉은-붉은 위반 상태 처리
  while (z->parent->color == RBTREE_RED) {
    // 2) 부모, 조부모, 삼촌 포인터 가져오기
    node_t *p = z->parent;        // 부모 노드
    node_t *g = p->parent;        // 조부모 노드

    // 3) Case 구분: 부모가 조부모의 왼쪽 자식인지 확인
    if (p == g->left) {
      node_t *u = g->right;       // 삼촌(uncle) 노드

      if (u->color == RBTREE_RED) {
        // === Case 1: 삼촌도 RED인 경우 ===
        // 부모와 삼촌을 BLACK으로, 조부모를 RED로 바꾼 뒤 조부모를 새로운 z로 삼아 위반 재검사
        p->color = RBTREE_BLACK;
        u->color = RBTREE_BLACK;
        g->color = RBTREE_RED;
        z = g;                     // 위반 검사용 노드를 조부모로 이동
      } else {
        // === Case 2: 삼촌이 BLACK이고 z가 부모의 오른쪽 자식일 때 ===
        if (z == p->right) {
          // z를 부모 위치로 올리고 좌회전으로 좌-우 불균형 변환
          z = p;
          rb_left_rotation(t, z);
          // 갱신: 새 부모, 새 조부모
          p = z->parent;
          g = p->parent;
        }
        // === Case 3: 삼촌이 BLACK이고 z가 부모의 왼쪽 자식일 때 ===
        // 부모를 BLACK, 조부모를 RED로 색상 변경 후 우회전 수행
        p->color = RBTREE_BLACK;
        g->color = RBTREE_RED;
        rb_right_rotation(t, g);
      }
    } else {
      // === Mirror Case: 부모가 조부모의 오른쪽 자식인 경우 ===
      node_t *u = g->left;        // 삼촌(uncle) = 조부모의 왼쪽

      if (u->color == RBTREE_RED) {
        // Mirror Case 1: 삼촌도 RED
        p->color = RBTREE_BLACK;
        u->color = RBTREE_BLACK;
        g->color = RBTREE_RED;
        z = g;
      } else {
        // Mirror Case 2: 삼촌 BLACK & z가 부모의 왼쪽 자식
        if (z == p->left) {
          z = p;
          rb_right_rotation(t, z);
          p = z->parent;
          g = p->parent;
        }
        // Mirror Case 3: 삼촌 BLACK & z가 부모의 오른쪽 자식
        p->color = RBTREE_BLACK;
        g->color = RBTREE_RED;
        rb_left_rotation(t, g);
      }
    }
  }

  // 4) 모든 위반을 해결한 후, 트리의 루트는 반드시 BLACK으로 유지
  t->root->color = RBTREE_BLACK;
}

// --- 좌회전(Rotate Left) ---
void rb_left_rotation(rbtree *t, node_t *current) {
  // 1) 회전 대상의 오른쪽 자식을 저장
  node_t *right_child = current->right;

  // 2) current 의 오른쪽을 right_child 의 왼쪽 서브트리로 연결
  current->right = right_child->left;

  // 3) 옮겨진 서브트리가 nil이 아니면, 해당 노드의 부모를 current로 갱신
  if (right_child->left != t->nil) {
    right_child->left->parent = current;
  }

  // 4) right_child 를 current 의 부모와 연결
  right_child->parent = current->parent;

  // 5) current 가 루트였다면, new root를 right_child로 업데이트
  if (current->parent == t->nil) {
    t->root = right_child;
  }
  // 6) current 가 부모의 왼쪽 자식이면, 부모의 왼쪽 포인터 갱신
  else if (current == current->parent->left) {
    current->parent->left = right_child;
  }
  // 7) 그렇지 않으면 우측 자식이었으므로, 부모의 오른쪽 포인터 갱신
  else {
    current->parent->right = right_child;
  }

  // 8) current 를 right_child 의 왼쪽 자식으로 연결
  right_child->left = current;
  // 9) current 의 부모를 right_child로 설정
  current->parent = right_child;
}


// --- 우회전(Rotate Right) ---
void rb_right_rotation(rbtree *t, node_t *current) {
  // 1) 회전 대상의 왼쪽 자식을 저장
  node_t *left_child = current->left;

  // 2) current 의 왼쪽을 left_child 의 오른쪽 서브트리로 연결
  current->left = left_child->right;

  // 3) 옮겨진 서브트리가 nil이 아니면, 해당 노드의 부모를 current로 갱신
  if (left_child->right != t->nil) {
    left_child->right->parent = current;
  }

  // 4) left_child 를 current 의 부모와 연결
  left_child->parent = current->parent;

  // 5) current 가 루트였다면, new root를 left_child로 업데이트
  if (current->parent == t->nil) {
    t->root = left_child;
  }
  // 6) current 가 부모의 오른쪽 자식이면, 부모의 오른쪽 포인터 갱신
  else if (current == current->parent->right) {
    current->parent->right = left_child;
  }
  // 7) 그 외에는 부모의 왼쪽 포인터 갱신
  else {
    current->parent->left = left_child;
  }

  // 8) current 를 left_child 의 오른쪽 자식으로 연결
  left_child->right = current;
  // 9) current 의 부모를 left_child로 설정
  current->parent = left_child;
}

node_t *rbtree_find(const rbtree *t, const key_t key) {
  // 탐색을 시작할 현재 노드를 root로 설정
  node_t *cur = t->root;

  // nil 노드에 도달할 때까지 반복
  while (t->nil != cur) {
    if (key == cur->key) {
      return cur; // 값을 찾으면 해당 노드 반환
    } else if (key < cur->key) {
      cur = cur->left;  // key가 작으면 왼쪽으로 이동
    } else {
      cur = cur->right; // key가 크면 오른쪽으로 이동
    }
  }

  // key를 찾지 못한 경우 NULL 반환
  return NULL;
}

node_t *rbtree_min(const rbtree *t) {
  node_t *cur = t->root;

  // 왼쪽 자식이 nil이 아닐 때까지 반복
  while (cur->left != t->nil) {
    cur = cur->left; // 더 왼쪽 자식으로 이동
  }

  // 가장 왼쪽(작은 값) 노드 반환
  return cur;
}

node_t *rbtree_max(const rbtree *t) {
  node_t *cur = t->root;

  // 오른쪽 자식이 nil이 아닐 때까지 반복
  while (cur->right != t->nil) {
    cur = cur->right; // 더 오른쪽 자식으로 이동
  }

  // 가장 오른쪽(큰 값) 노드 반환
  return cur;
}

int rbtree_erase(rbtree *t, node_t *p) {
  node_t *successor = p;
  node_t *replacement;
  color_t original_color = p->color;

  if (p->left == t->nil) {
    replacement = p->right;
    rb_node_change(t, p, replacement);
  } else if (p->right == t->nil) {
    replacement = p->left;
    rb_node_change(t, p, replacement);
  } else {
      successor = rb_tree_min_subtree(t, p->right);
      original_color = successor->color;
      replacement = successor->right;
      if (successor->parent == p) {
        replacement->parent = successor;
        successor->right = p->right;
        successor->right->parent = successor;

      }else {
        rb_node_change(t, successor, replacement);
        successor->right = p->right;
        successor->right->parent = successor;
      }
      rb_node_change(t, p, successor);
      successor->left = p->left;
      successor->left->parent = successor;
      successor->color = p->color;
    }
  
  if (original_color == RBTREE_BLACK) {
    rb_delete_fixup(t, replacement);
  }

  free(p);

  return 0;
}

void rb_delete_fixup(rbtree *t, node_t *target) {
  while (target != t->root && target->color == RBTREE_BLACK) {
      node_t *node_parent = target->parent;
      node_t *uncle;
      if (target == node_parent->left) {
          uncle = node_parent->right;
          if (uncle->color == RBTREE_RED) { 
              uncle->color = RBTREE_BLACK;
              target->parent->color = RBTREE_RED;
              rb_left_rotation(t, target->parent);
              uncle = target->parent->right;
          }else {
              if (uncle->left->color == RBTREE_BLACK && uncle->right->color == RBTREE_BLACK) {
                  uncle->color = RBTREE_RED;
                  target = target->parent;
              } else {
       
                  if (uncle->right->color == RBTREE_BLACK) {
                      uncle->left->color = RBTREE_BLACK;
                      uncle->color = RBTREE_RED;
                      rb_right_rotation(t,uncle);
                      uncle = node_parent->right;
                  }
            
                  uncle->color = node_parent->color;
                  node_parent->color = RBTREE_BLACK;
                  uncle->right->color = RBTREE_BLACK;
                  rb_left_rotation(t, node_parent);
                  target = t->root;

              }
          }

      }
      else {
          uncle = node_parent->left;
          if (uncle->color == RBTREE_RED) { 
              uncle->color = RBTREE_BLACK;
              target->parent->color = RBTREE_RED;
              rb_right_rotation(t, target->parent);
              uncle = target->parent->left;
          }else {
              if (uncle->left->color == RBTREE_BLACK && uncle->right->color == RBTREE_BLACK) {
                  uncle->color = RBTREE_RED;
                  target = target->parent;
              } else {
                  if (uncle->left->color == RBTREE_BLACK) {
                      uncle->right->color = RBTREE_BLACK;
                      uncle->color = RBTREE_RED;
                      rb_left_rotation(t,uncle);
                      uncle = node_parent->left;
                  }
                  uncle->color = node_parent->color;
                  node_parent->color = RBTREE_BLACK;
                  uncle->left->color = RBTREE_BLACK;
                  rb_right_rotation(t, node_parent);
                  target = t->root;
              }
          }
      }
  }
  target->color = RBTREE_BLACK;

}

void rb_node_change(rbtree *t, node_t *p, node_t *replacement) {
  if(p->parent == t->nil) {
    t->root = replacement;
  } else if (p == p->parent->left) {
    p->parent->left = replacement;
  } else {
    p->parent->right = replacement;
  }
  replacement->parent = p->parent;
}

node_t *rb_tree_min_subtree(rbtree *t, node_t *start) {
  node_t *cur = start;
  while (cur->left != t->nil) {
    cur = cur->left;
  }
  return cur;
}

void inorder_search(const rbtree *t, key_t *arr, size_t n, int *cur_i, node_t *node){
  // nil 노드이거나 배열이 다 찼으면 종료
  if (node == t->nil || *cur_i >= n) {
    return;
  }

  // 왼쪽 서브트리 순회
  inorder_search(t, arr, n, cur_i, node->left);

  // 현재 노드의 key 저장
  if (*cur_i < n) {
    arr[*cur_i] = node->key;
    *cur_i += 1; // 인덱스 증가
  }

  // 오른쪽 서브트리 순회
  inorder_search(t, arr, n, cur_i, node->right);
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  int index = 0;

  // 중위 순회를 통해 key들을 오름차순으로 배열에 저장
  inorder_search(t, arr, n, &index, t->root);

  // 저장한 키 개수 반환
  return index;
}
