#include "seed_buckets.h"

class test_node : public bucket_node<test_node>{
};

template class seed_buckets<test_node>;