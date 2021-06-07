#ifndef REIMPLS_HPF_H__
#define REIMPLS_HPF_H__

/* LICENSE
 *
 * The source code is subject to the following academic license.
 * Note this is not an open source license.
 *
 * Copyright © 2001. The Regents of the University of California (Regents).
 * All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research, and not-for-profit purposes,
 * without fee and without a signed licensing agreement, is hereby granted,
 * provided that the above copyright notice, this paragraph and the following
 * two paragraphs appear in all copies, modifications, and distributions.
 * Contact The Office of Technology Licensing, UC Berkeley, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for commercial
 * licensing opportunities. Created by Bala Chandran and Dorit S. Hochbaum,
 * Department of Industrial Engineering and Operations Research,
 * University of California, Berkeley.
 *
 * IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
 * SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
 * REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
 * HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE
 * MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#include <vector>
#include <cinttypes>

namespace reimpls {

enum class LabelOrder {
    HIGHEST_FIRST,
    LOWEST_FIRST
};

enum class RootOrder {
    FIFO,
    LIFO
};

template <class Cap, LabelOrder LABEL_ORDER = LabelOrder::HIGHEST_FIRST,
    RootOrder ROOT_ORDER = RootOrder::FIFO>
class Hpf {
    // Forward decls.
    struct Node;
    struct Arc;
    struct Root;

public:
    enum TermType : uint32_t {
        SOURCE = 0,
        SINK = 1
    };

    Hpf(size_t expectedNodes = 0, size_t expectedArcs = 0);

    uint32_t add_node(uint32_t num = 1);

    void add_edge(uint32_t from, uint32_t to, Cap capacity);

    void mincut();

    TermType what_label(uint32_t node) const;
    Cap compute_maxflow() const noexcept;
    void recover_flow();

    inline void set_source(uint32_t s) { source = s; }
    inline void set_sink(uint32_t t) { sink = t; }

private:
    uint32_t numNodes;
    uint32_t numArcs;
    uint32_t source;
    uint32_t sink;

    uint32_t first;
    uint32_t last;

    uint32_t highestStrongLabel;
    uint32_t lowestStrongLabel;

    std::vector<Node> adjacencyList;
    std::vector<Root> strongRoots;
    std::vector<uint32_t> labelCount;
    std::vector<Arc> arcList;
    std::vector<Arc *> outOfTreePtrs;

    void init_mincut();

    uint32_t gap() const noexcept;
    void decompose(Node *excessNode, const uint32_t source, uint32_t *iteration);

    void sort(Node *current);
    void minisort(Node *current);
    void quickSort(Arc **arr, const uint32_t first, const uint32_t last);

    void addToStrongBucket(Node *newRoot, Root *rootBucket);

    Node *getHighestStrongRoot();
    Node *getLowestStrongRoot();
    Node *getNextStrongRoot();

    void processRoot(Node *strongRoot);

    Arc *findWeakNode(Node *strongNode, Node **weakNode);

    void merge(Node *parent, Node *child, Arc *newArc);
    void addRelationship(Node *newParent, Node *child);
    void breakRelationship(Node *oldParent, Node *child);

    void pushExcess(Node *strongRoot);
    void pushUpward(Arc *currentArc, Node *child, Node *parent, Cap resCap);
    void pushDownward(Arc *currentArc, Node *child, Node *parent, Cap flow);

    void checkChildren(Node *curNode);

    void liftAll(Node *rootNode);

    struct Node {
        uint32_t visited;
        uint32_t numAdjacent;
        uint32_t number;
        uint32_t label;
        Cap excess;

        Node *parent;
        Node *childList;
        Node *nextScan;

        uint32_t numOutOfTree;
        Arc **outOfTree;
        uint32_t nextArc;
        Arc *arcToParent;
        Node *next;

        Node() :
            visited(0),
            numAdjacent(0),
            number(0),
            label(0),
            excess(0),
            parent(nullptr),
            childList(nullptr),
            nextScan(nullptr),
            numOutOfTree(0),
            outOfTree(nullptr),
            nextArc(0),
            arcToParent(nullptr),
            next(nullptr) {}

        void addOutOfTree(Arc *out);
    };

    struct Arc {
        Node *from;
        Node *to;
        Cap flow;
        Cap capacity;
        bool direction;

        Arc(Node *from, Node *to, Cap capacity) :
            from(from),
            to(to),
            flow(0),
            capacity(capacity),
            direction(true) {}
     };

    struct Root {
        Node *start;
        Node *end;

        Root() :
            start(nullptr),
            end(nullptr) {}
    };
};

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::Hpf(size_t expectedNodes, size_t expectedArcs) :
    numNodes(0),
    numArcs(0),
    source(0),
    sink(0),
    first(0),
    last(0),
    highestStrongLabel(1),
    lowestStrongLabel(1),
    adjacencyList(),
    strongRoots(),
    labelCount(),
    arcList(),
    outOfTreePtrs()
{
    adjacencyList.reserve(expectedNodes);
    strongRoots.reserve(expectedNodes);
    labelCount.reserve(expectedNodes);
    arcList.reserve(expectedArcs);
    outOfTreePtrs.reserve(expectedArcs * 2);
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline uint32_t Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::add_node(uint32_t num)
{
    numNodes += num;
    adjacencyList.resize(numNodes);
    strongRoots.resize(numNodes);
    labelCount.resize(numNodes, 0);
    return numNodes;
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline void Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::add_edge(uint32_t from, uint32_t to, Cap capacity)
{
    arcList.emplace_back(
        &adjacencyList[from],
        &adjacencyList[to],
        capacity
    );
    adjacencyList[from].numAdjacent++;
    adjacencyList[to].numAdjacent++;
    numArcs++;
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline void Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::mincut()
{
    init_mincut();

    // pseudoflowPhase1
    Node *strongRoot;

    while (strongRoot = getNextStrongRoot()) {
        processRoot(strongRoot);
    }
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline typename Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::TermType Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::what_label(
    uint32_t node) const
{
    return adjacencyList[node].label >= gap ? SOURCE : SINK;
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline Cap Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::compute_maxflow() const noexcept
{
    Cap cut = 0;

    // Compute value of minimum cut which is equal to the max. flow
    for (const Arc& a : arcList) {
        if (a.from->label >= gap() && a.to->label < gap()) {
            cut += a.capacity;
        }
    }

    return cut;
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline void Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::recover_flow()
{
    uint32_t i, j, iteration = 1;
    Arc *tempArc;
    Node *tempNode;

    for (i = 0; i < adjacencyList[sink].numOutOfTree; ++i) {
        tempArc = adjacencyList[sink].outOfTree[i];
        if (tempArc->from->excess < 0) {
            if ((tempArc->from->excess + (int)tempArc->flow) < 0) {
                tempArc->from->excess += (int)tempArc->flow;
                tempArc->flow = 0;
            } else {
                tempArc->flow = (uint32_t)(tempArc->from->excess + (int)tempArc->flow);
                tempArc->from->excess = 0;
            }
        }
    }

    for (i = 0; i < adjacencyList[source].numOutOfTree; ++i) {
        tempArc = adjacencyList[source].outOfTree[i];
        //addOutOfTreeNode(tempArc->to, tempArc);
        tempArc->to->addOutOfTree(tempArc);
    }

    adjacencyList[source].excess = 0;
    adjacencyList[sink].excess = 0;

    for (i = 0; i < numNodes; ++i) {
        tempNode = &adjacencyList[i];

        if (i == source - 1 || i == sink - 1) {
            continue;
        }

        if (tempNode->label >= gap()) {
            tempNode->nextArc = 0;
            if ((tempNode->parent) && (tempNode->arcToParent->flow)) {
                //addOutOfTreeNode(tempNode->arcToParent->to, tempNode->arcToParent);
                tempNode->arcToParent->to->addOutOfTree(tempNode->arcToParent);
            }

            for (j = 0; j < tempNode->numOutOfTree; ++j) {
                if (!tempNode->outOfTree[j]->flow) {
                    --tempNode->numOutOfTree;
                    tempNode->outOfTree[j] = tempNode->outOfTree[tempNode->numOutOfTree];
                    --j;
                }
            }

            sort(tempNode);
        }
    }

    for (i = 0; i < numNodes; ++i) {
        tempNode = &adjacencyList[i];
        while (tempNode->excess > 0) {
            ++iteration;
            decompose(tempNode, source, &iteration);
        }
    }
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline void Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::init_mincut()
{
    outOfTreePtrs.resize(2 * arcList.size());
    Arc **crntOutOfTree = outOfTreePtrs.data();
    for (uint32_t i = 0; i < numNodes; ++i) {
        Node& n = adjacencyList[i];
        n.number = i;
        // createOutOfTree
        if (n.numAdjacent) {
            n.outOfTree = crntOutOfTree;
            crntOutOfTree += n.numAdjacent;
        }
    }

    for (Arc& a : arcList) {
        uint32_t from = a.from->number;
        uint32_t to = a.to->number;
        if (!(source == to || sink == from || from == to)) {
            if (source == from && to == sink) {
                a.flow = a.capacity;
            } else if (from == source) {
                a.from->addOutOfTree(&a);
            } else if (to == sink) {
                a.to->addOutOfTree(&a);
            } else {
                a.from->addOutOfTree(&a);
            }
        }
    }

    // simpleInitialization
    for (uint32_t i = 0; i < adjacencyList[source].numOutOfTree; ++i) {
        Arc *tempArc = adjacencyList[source].outOfTree[i];
        tempArc->flow = tempArc->capacity;
        tempArc->to->excess += tempArc->capacity;
    }

    for (uint32_t i = 0; i < adjacencyList[sink].numOutOfTree; ++i) {
        Arc *tempArc = adjacencyList[sink].outOfTree[i];
        tempArc->flow = tempArc->capacity;
        tempArc->from->excess -= tempArc->capacity;
    }

    adjacencyList[source].excess = 0;
    adjacencyList[sink].excess = 0;

    for (uint32_t i = 0; i < numNodes; ++i) {
        if (adjacencyList[i].excess > 0) {
            adjacencyList[i].label = 1;
            ++labelCount[1];

            addToStrongBucket(&adjacencyList[i], &strongRoots[1]);
        }
    }

    adjacencyList[source].label = numNodes;
    adjacencyList[sink].label = 0;
    labelCount[0] = (numNodes - 2) - labelCount[1];
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline uint32_t Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::gap() const noexcept
{
    return LABEL_ORDER == LabelOrder::LOWEST_FIRST ? lowestStrongLabel : numNodes;
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline void Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::decompose(
    Node * excessNode, const uint32_t source, uint32_t * iteration)
{
    Node *current = excessNode;
    Arc *tempArc;
    Cap bottleneck = excessNode->excess;

    for (; current->number != source && current->visited < (*iteration); current = tempArc->from) {
        current->visited = (*iteration);
        tempArc = current->outOfTree[current->nextArc];

        if (tempArc->flow < bottleneck) {
            bottleneck = tempArc->flow;
        }
    }

    if (current->number == source) {
        excessNode->excess -= bottleneck;
        current = excessNode;

        while (current->number != source) {
            tempArc = current->outOfTree[current->nextArc];
            tempArc->flow -= bottleneck;

            if (tempArc->flow) {
                minisort(current);
            } else {
                ++current->nextArc;
            }
            current = tempArc->from;
        }
        return;
    }

    ++(*iteration);

    bottleneck = current->outOfTree[current->nextArc]->flow;

    while (current->visited < (*iteration)) {
        current->visited = (*iteration);
        tempArc = current->outOfTree[current->nextArc];

        if (tempArc->flow < bottleneck) {
            bottleneck = tempArc->flow;
        }
        current = tempArc->from;
    }

    ++(*iteration);

    while (current->visited < (*iteration)) {
        current->visited = (*iteration);

        tempArc = current->outOfTree[current->nextArc];
        tempArc->flow -= bottleneck;

        if (tempArc->flow) {
            minisort(current);
            current = tempArc->from;
        } else {
            ++current->nextArc;
            current = tempArc->from;
        }
    }
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline void Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::sort(Node * current)
{
    // TODO: Just use the sort algorithm from the standard, since that's also quicksort
    if (current->numOutOfTree > 1) {
        quickSort(current->outOfTree, 0, (current->numOutOfTree - 1));
    }
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline void Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::minisort(Node * current)
{
    Arc *temp = current->outOfTree[current->nextArc];
    uint32_t i, size = current->numOutOfTree, tempflow = temp->flow;

    for (i = current->nextArc + 1; i < size && tempflow < current->outOfTree[i]->flow; ++i) {
        current->outOfTree[i - 1] = current->outOfTree[i];
    }
    current->outOfTree[i - 1] = temp;
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline void Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::quickSort(
    Arc * *arr, const uint32_t first, const uint32_t last)
{
    uint32_t i, j, left = first, right = last, x1, x2, x3, mid, pivot, pivotval;
    Arc *swap;

    if ((right - left) <= 5) {// Bubble sort if 5 elements or less
        for (i = right; (i > left); --i) {
            swap = nullptr;
            for (j = left; j < i; ++j) {
                if (arr[j]->flow < arr[j + 1]->flow) {
                    swap = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = swap;
                }
            }

            if (!swap) {
                return;
            }
        }

        return;
    }

    mid = (first + last) / 2;

    x1 = arr[first]->flow;
    x2 = arr[mid]->flow;
    x3 = arr[last]->flow;

    pivot = mid;

    if (x1 <= x2) {
        if (x2 > x3) {
            pivot = left;

            if (x1 <= x3) {
                pivot = right;
            }
        }
    } else {
        if (x2 <= x3) {
            pivot = right;

            if (x1 <= x3) {
                pivot = left;
            }
        }
    }

    pivotval = arr[pivot]->flow;

    swap = arr[first];
    arr[first] = arr[pivot];
    arr[pivot] = swap;

    left = (first + 1);

    while (left < right) {
        if (arr[left]->flow < pivotval) {
            swap = arr[left];
            arr[left] = arr[right];
            arr[right] = swap;
            --right;
        } else {
            ++left;
        }
    }

    swap = arr[first];
    arr[first] = arr[left];
    arr[left] = swap;

    if (first < (left - 1)) {
        quickSort(arr, first, (left - 1));
    }

    if ((left + 1) < last) {
        quickSort(arr, (left + 1), last);
    }
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline void Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::addToStrongBucket(Node *newRoot, Root *rootBucket)
{
    if (ROOT_ORDER == RootOrder::FIFO) {
        if (rootBucket->start) {
            rootBucket->end->next = newRoot;
            rootBucket->end = newRoot;
            newRoot->next = nullptr;
        } else {
            rootBucket->start = newRoot;
            rootBucket->end = newRoot;
            newRoot->next = nullptr;
        }
    } else {
        newRoot->next = rootBucket->start;
        rootBucket->start = newRoot;
    }
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline typename Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::Node *
Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::getHighestStrongRoot()
{
    uint32_t i;
    Node *strongRoot;

    for (i = highestStrongLabel; i > 0; --i) {
        if (strongRoots[i].start) {
            highestStrongLabel = i;
            if (labelCount[i - 1]) {
                strongRoot = strongRoots[i].start;
                strongRoots[i].start = strongRoot->next;
                strongRoot->next = nullptr;
                return strongRoot;
            }

            while (strongRoots[i].start) {
                strongRoot = strongRoots[i].start;
                strongRoots[i].start = strongRoot->next;
                liftAll(strongRoot);
            }
        }
    }

    if (!strongRoots[0].start) {
        return nullptr;
    }

    while (strongRoots[0].start) {
        strongRoot = strongRoots[0].start;
        strongRoots[0].start = strongRoot->next;
        strongRoot->label = 1;
        --labelCount[0];
        ++labelCount[1];

        addToStrongBucket(strongRoot, &strongRoots[strongRoot->label]);
    }

    highestStrongLabel = 1;

    strongRoot = strongRoots[1].start;
    strongRoots[1].start = strongRoot->next;
    strongRoot->next = nullptr;

    return strongRoot;
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline typename Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::Node *
Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::getLowestStrongRoot()
{
    uint32_t i;
    Node *strongRoot;

    if (lowestStrongLabel == 0) {
        while (strongRoots[0].start) {
            strongRoot = strongRoots[0].start;
            strongRoots[0].start = strongRoot->next;
            strongRoot->next = nullptr;

            strongRoot->label = 1;

            --labelCount[0];
            ++labelCount[1];

            addToStrongBucket(strongRoot, &strongRoots[strongRoot->label]);
        }
        lowestStrongLabel = 1;
    }

    for (i = lowestStrongLabel; i < numNodes; ++i) {
        if (strongRoots[i].start) {
            lowestStrongLabel = i;

            if (labelCount[i - 1] == 0) {
                return nullptr;
            }

            strongRoot = strongRoots[i].start;
            strongRoots[i].start = strongRoot->next;
            strongRoot->next = nullptr;
            return strongRoot;
        }
    }

    lowestStrongLabel = numNodes;
    return nullptr;
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline typename Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::Node *
Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::getNextStrongRoot()
{
    if (LABEL_ORDER == LabelOrder::LOWEST_FIRST) {
        return getLowestStrongRoot();
    } else {
        return getHighestStrongRoot();
    }
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline void Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::processRoot(Node *strongRoot)
{
    Node *temp, *strongNode = strongRoot, *weakNode;
    Arc *out;

    strongRoot->nextScan = strongRoot->childList;

    if ((out = findWeakNode(strongRoot, &weakNode))) {
        merge(weakNode, strongNode, out);
        pushExcess(strongRoot);
        return;
    }

    checkChildren(strongRoot);

    while (strongNode) {
        while (strongNode->nextScan) {
            temp = strongNode->nextScan;
            strongNode->nextScan = strongNode->nextScan->next;
            strongNode = temp;
            strongNode->nextScan = strongNode->childList;

            if ((out = findWeakNode(strongNode, &weakNode))) {
                merge(weakNode, strongNode, out);
                pushExcess(strongRoot);
                return;
            }

            checkChildren(strongNode);
        }

        if ((strongNode = strongNode->parent)) {
            checkChildren(strongNode);
        }
    }

    addToStrongBucket(strongRoot, &strongRoots[strongRoot->label]);

    if (LABEL_ORDER == LabelOrder::HIGHEST_FIRST) {
        ++highestStrongLabel;
    }
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline typename Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::Arc *Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::findWeakNode(
    Node *strongNode, Node **weakNode)
{
    uint32_t i, size;
    Arc *out;

    const uint32_t extremumStrongLabel = LABEL_ORDER == LabelOrder::LOWEST_FIRST ?
        lowestStrongLabel : highestStrongLabel;
    size = strongNode->numOutOfTree;

    for (i = strongNode->nextArc; i < size; ++i) {
        if (strongNode->outOfTree[i]->to->label == (extremumStrongLabel - 1)) {
            strongNode->nextArc = i;
            out = strongNode->outOfTree[i];
            (*weakNode) = out->to;
            --strongNode->numOutOfTree;
            strongNode->outOfTree[i] = strongNode->outOfTree[strongNode->numOutOfTree];
            return out;
        } else if (strongNode->outOfTree[i]->from->label == (extremumStrongLabel - 1)) {
            strongNode->nextArc = i;
            out = strongNode->outOfTree[i];
            (*weakNode) = out->from;
            --strongNode->numOutOfTree;
            strongNode->outOfTree[i] = strongNode->outOfTree[strongNode->numOutOfTree];
            return out;
        }
    }

    strongNode->nextArc = strongNode->numOutOfTree;

    return nullptr;
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline void Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::merge(Node *parent, Node *child, Arc *newArc)
{
    Arc *oldArc;
    Node *current = child, *oldParent, *newParent = parent;

    while (current->parent) {
        oldArc = current->arcToParent;
        current->arcToParent = newArc;
        oldParent = current->parent;
        breakRelationship(oldParent, current);
        addRelationship(newParent, current);
        newParent = current;
        current = oldParent;
        newArc = oldArc;
        newArc->direction = 1 - newArc->direction;
    }

    current->arcToParent = newArc;
    addRelationship(newParent, current);
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline void Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::addRelationship(Node * newParent, Node * child)
{
    child->parent = newParent;
    child->next = newParent->childList;
    newParent->childList = child;
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline void Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::breakRelationship(Node * oldParent, Node * child)
{
    Node *current;

    child->parent = nullptr;

    if (oldParent->childList == child) {
        oldParent->childList = child->next;
        child->next = nullptr;
        return;
    }

    for (current = oldParent->childList; current->next != child; current = current->next) {
        // Do nothing
    }

    current->next = child->next;
    child->next = nullptr;
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline void Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::pushExcess(Node *strongRoot)
{
    Node *current, *parent;
    Arc *arcToParent;
    int prevEx = 1;

    for (current = strongRoot; current->excess && current->parent; current = parent) {
        parent = current->parent;
        prevEx = parent->excess;

        arcToParent = current->arcToParent;

        if (arcToParent->direction) {
            pushUpward(arcToParent, current, parent, (arcToParent->capacity - arcToParent->flow));
        } else {
            pushDownward(arcToParent, current, parent, arcToParent->flow);
        }
    }

    if ((current->excess > 0) && (prevEx <= 0)) {
        if (LABEL_ORDER == LabelOrder::LOWEST_FIRST) {
            lowestStrongLabel = current->label;
        }
        addToStrongBucket(current, &strongRoots[current->label]);
    }
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline void Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::pushUpward(
    Arc *currentArc, Node *child, Node *parent, Cap resCap)
{
    if (resCap >= child->excess) {
        parent->excess += child->excess;
        currentArc->flow += child->excess;
        child->excess = 0;
        return;
    }

    currentArc->direction = 0;
    parent->excess += resCap;
    child->excess -= resCap;
    currentArc->flow = currentArc->capacity;
    parent->outOfTree[parent->numOutOfTree] = currentArc;
    ++parent->numOutOfTree;
    breakRelationship(parent, child);

    if (LABEL_ORDER == LabelOrder::LOWEST_FIRST) {
        lowestStrongLabel = child->label;
    }

    addToStrongBucket(child, &strongRoots[child->label]);
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline void Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::pushDownward(
    Arc *currentArc, Node *child, Node *parent, Cap flow)
{
    if (flow >= child->excess) {
        parent->excess += child->excess;
        currentArc->flow -= child->excess;
        child->excess = 0;
        return;
    }

    currentArc->direction = 1;
    child->excess -= flow;
    parent->excess += flow;
    currentArc->flow = 0;
    parent->outOfTree[parent->numOutOfTree] = currentArc;
    ++parent->numOutOfTree;
    breakRelationship(parent, child);

    if (LABEL_ORDER == LabelOrder::LOWEST_FIRST) {
        lowestStrongLabel = child->label;
    }

    addToStrongBucket(child, &strongRoots[child->label]);
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline void Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::checkChildren(Node *curNode)
{
    for (; curNode->nextScan; curNode->nextScan = curNode->nextScan->next) {
        if (curNode->nextScan->label == curNode->label) {
            return;
        }
    }

    --labelCount[curNode->label];
    ++curNode->label;
    ++labelCount[curNode->label];

    curNode->nextArc = 0;
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline void Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::liftAll(Node *rootNode)
{
    Node *temp, *current = rootNode;

    current->nextScan = current->childList;

    --labelCount[current->label];
    current->label = numNodes;

    for (; (current); current = current->parent) {
        while (current->nextScan) {
            temp = current->nextScan;
            current->nextScan = current->nextScan->next;
            current = temp;
            current->nextScan = current->childList;

            --labelCount[current->label];
            current->label = numNodes;
        }
    }
}

template <class Cap, LabelOrder LABEL_ORDER, RootOrder ROOT_ORDER>
inline void Hpf<Cap, LABEL_ORDER, ROOT_ORDER>::Node::addOutOfTree(Arc *out)
{
    outOfTree[numOutOfTree] = out;
    numOutOfTree++;
}

} // namespace reimpls

#endif // REIMPLS_HPF_H__