#include <iostream>
#include <algorithm>
#include <functional>
#include <random>
#include <set>
#include <stdexcept>
#include <boost/timer/timer.hpp>

using namespace std;
using u8 = unsigned char;
class Rigion {
    u8 *rigion;
    size_t cap, index;
public:
    Rigion(const size_t size): rigion(new u8[size]), cap(size), index(0) {}
    ~Rigion() {delete[] rigion;}
    // void *Alloc(const size_t size) {
    //     void *ret = (void *)(rigion+index);
    //     index += size;
    //     if(index > cap)
    //         throw bad_alloc();
    //     return ret;
    // }
    // static void *Alloc(Rigion &r, const size_t size) {
    //     void *ret = (void *)(r.rigion+r.index);
    //     r.index += size;
    //     if(r.index > r.cap)
    //         throw bad_alloc();
    //     return ret;
    // }
    static void *Alloc(Rigion *r, const size_t size) {
        void *ret = (void *)(r->rigion+r->index);
        r->index += size;
        if(r->index > r->cap)
            throw bad_alloc();
        return ret;
    }
};

class Avl {
    struct Node {
        int level;
        int num;
        Node *l, *r;
        Node *LL() {
            Node *ret = l;
            l = ret->r;
            ret->r = this;
            level = max(l ? l->level : 0, r ? r->level : 0) + 1;
            ret->level = max(ret->l->level, level) + 1;
            return ret;
        }
        Node *LR() {
            Node *ret = l->r;
            l->r = ret->l;
            ret->l = l;
            l = ret->r;
            ret->r = this;
            level = max(l ? l->level : 0, r ? r->level : 0) + 1;
            ret->level = max(ret->l->level, level) + 1;
            return ret;
        }
        Node *RR() {
            Node *ret = r;
            r = ret->l;
            ret->l = this;
            level = max(l ? l->level : 0, r ? r->level : 0) + 1;
            ret->level = max(ret->r->level, level) + 1;
            return ret;
        }
        Node *RL() {
            Node *ret = r->l;
            r->l = ret->r;
            ret->r = r;
            r = ret->l;
            ret->l = this;
            level = max(l ? l->level : 0, r ? r->level : 0) + 1;
            ret->level = max(ret->r->level, level) + 1;
            return ret;
        }
        Node *Emplace(int insert, function<void*(const size_t)> &alloc) {
            if(insert == num)
                return this;

            if(insert < num) {
                if(l == nullptr) {
                    l = (Node*)alloc(sizeof(Node));
                    new(l) Node{1, insert, nullptr, nullptr};
                } else
                    l = l->Emplace(insert, alloc);
            } else {
                if(r == nullptr) {
                    r = (Node*)alloc(sizeof(Node));
                    new(r) Node{1, insert, nullptr, nullptr};
                } else
                    r = r->Emplace(insert, alloc);
            }

            int lv = (l ? l->level : 0 ), rv = (r ? r->level : 0 );
            if(abs(lv - rv) <= 1) {
                level = max(lv, rv) + 1;
                return this;
            }

            if(lv > rv) {
                int llv = (l->l ? l->l->level : 0), lrv = (l->r ? l->r->level : 0);
                if(llv > lrv)
                    return LL();
                else
                    return LR();
            } else {
                int rrv = (r->r ? r->r->level : 0), rlv = (r->l ? r->l->level : 0);
                if(rrv > rlv)
                    return RR();
                else
                    return RL();
            }
        }
        static bool Find(Node *head, int num) {
            while(head != nullptr) {
                if(num == head->num)
                    return true;
                else if(num < head->num)
                    head = head->l;
                else
                    head = head->r;
            }
            return false;
        }
    };
    function<void*(const size_t)> alloc;
    Node *head;
public:
    Avl(auto allocfunc): alloc(allocfunc), head(nullptr) {}
    void Emplace(int num) {
        if(head == nullptr) {
            head = (Node*)alloc(sizeof(Node));
            new(head) Node{1, num, nullptr, nullptr};
        } else {
            head = head->Emplace(num, alloc);
        }
    }
    bool Find(int num) {
        return Node::Find(head, num);
    }
};

// const size_t CAPACITY = 40960;
// const size_t NODESIZE = 1000;
// const size_t LOOPSIZE = 10000;

// const size_t CAPACITY = 102400;
// const size_t NODESIZE = 2500;
// const size_t LOOPSIZE = 10000;
// const size_t DIVSIZE = 100;

const size_t CAPACITY = 409600;
const size_t NODESIZE = 10000;
const size_t LOOPSIZE = 1000;
const size_t DIVSIZE = 100;

int main() {
    using std::placeholders::_1;
    Rigion r1(CAPACITY), r2(CAPACITY), r3(CAPACITY), r4(CAPACITY);
    Avl a1(bind(Rigion::Alloc, &r1, _1)), a2(bind(Rigion::Alloc, &r2, _1)), a3(bind(Rigion::Alloc, &r3, _1)), a4(bind(Rigion::Alloc, &r4, _1));
    set<int> s1, s2, s3, s4;

    vector<int> nums1;
    vector<int> nums2;
    vector<int> nums3;
    vector<int> nums4;
    
    cout << " 0:" << endl;
    try {
        default_random_engine e(random_device{}());
        // for(size_t i = 0; i < NODESIZE; i++)
        //     nums1.emplace_back(e());
        uniform_int_distribution<int> uniform_dist(0, NODESIZE*3);
        for(size_t i = 0; i < NODESIZE; i++)
            nums1.emplace_back(uniform_dist(e));
        nums2 = nums1;
        sort(nums2.begin(), nums2.end());
        for(size_t i = 0; i < NODESIZE; i++)
            nums3.emplace_back(nums2[i/2+(i%2)*((NODESIZE+1)/2)]);
        for(size_t i = 0; i < NODESIZE; i++)
            nums4.emplace_back(nums2[i/DIVSIZE+(i%DIVSIZE)*((NODESIZE-1)/DIVSIZE+1)]);

        for(auto i: nums1)
            a1.Emplace(i);
        for(auto i: nums2)
            a2.Emplace(i);
        for(auto i: nums3)
            a3.Emplace(i);
        for(auto i: nums4)
            a4.Emplace(i);

        for(auto i: nums1)
            s1.emplace(i);
        for(auto i: nums2)
            s2.emplace(i);
        for(auto i: nums3)
            s3.emplace(i);
        for(auto i: nums4)
            s4.emplace(i);
        
        for(size_t i = 0, j = 0; i < NODESIZE*3; i++) {
            int add = a1.Find(i) + a2.Find(i) + a3.Find(i) + a4.Find(i)
                + (s1.find(i) != s1.end())
                + (s2.find(i) != s2.end())
                + (s3.find(i) != s3.end())
                + (s4.find(i) != s4.end());
            if(add != 0 && add != 8) {
                string errstr = string("i = ") + to_string(i)
                    + ", a1 = " + to_string(a1.Find(i))
                    + ", a2 = " + to_string(a2.Find(i))
                    + ", a3 = " + to_string(a3.Find(i))
                    + ", a4 = " + to_string(a4.Find(i))
                    + ", s1 = " + to_string(s1.find(i) != s1.end())
                    + ", s2 = " + to_string(s2.find(i) != s2.end())
                    + ", s3 = " + to_string(s3.find(i) != s3.end())
                    + ", s4 = " + to_string(s4.find(i) != s4.end());
                    
                std::cout << errstr;
                throw out_of_range(errstr);
            }
        }

    } catch (const std::exception& e) {
        std::cout << e.what();
        return 1;
    }

    cout << " 1:" << endl;
    try {
        boost::timer::cpu_timer t;
        t.start();
        for(size_t j = 0; j < LOOPSIZE; j++) {
            for(auto i: nums1)
                a2.Find(i);
            for(auto i: nums2)
                a2.Find(i);
            for(auto i: nums3)
                a2.Find(i);
        }
        t.stop();
        cout << "all : " << t.elapsed().wall/1000000 << "ms" << endl;   //输出：start()至调用此函数的经过时间
        cout << "user : " << t.elapsed().user/1000000 << "ms" << endl;   //输出：start()至调用此函数的用户时间
        cout << "system : " << t.elapsed().system/1000000 << "ms" << endl; //输出：start()至调用此函数的系统时间
    } catch (const std::exception& e) {
        std::cout << e.what();
        return 1;
    }
    cout << " 2:" << endl;
    try {
        boost::timer::cpu_timer t;
        t.start();
        for(size_t j = 0; j < LOOPSIZE; j++) {
            for(auto i: nums1)
                a1.Find(i);
            for(auto i: nums2)
                a1.Find(i);
            for(auto i: nums3)
                a1.Find(i);
        }
        t.stop();
        cout << "all : " << t.elapsed().wall/1000000 << "ms" << endl;   //输出：start()至调用此函数的经过时间
        cout << "user : " << t.elapsed().user/1000000 << "ms" << endl;   //输出：start()至调用此函数的用户时间
        cout << "system : " << t.elapsed().system/1000000 << "ms" << endl; //输出：start()至调用此函数的系统时间
    } catch (const std::exception& e) {
        std::cout << e.what();
        return 1;
    }
    cout << " 3:" << endl;
    try {
        boost::timer::cpu_timer t;
        t.start();
        for(size_t j = 0; j < LOOPSIZE; j++) {
            for(auto i: nums1)
                a3.Find(i);
            for(auto i: nums2)
                a3.Find(i);
            for(auto i: nums3)
                a3.Find(i);
        }
        t.stop();
        cout << "all : " << t.elapsed().wall/1000000 << "ms" << endl;   //输出：start()至调用此函数的经过时间
        cout << "user : " << t.elapsed().user/1000000 << "ms" << endl;   //输出：start()至调用此函数的用户时间
        cout << "system : " << t.elapsed().system/1000000 << "ms" << endl; //输出：start()至调用此函数的系统时间
    } catch (const std::exception& e) {
        std::cout << e.what();
        return 1;
    }
    cout << " 4:" << endl;
    try {
        boost::timer::cpu_timer t;
        t.start();
        for(size_t j = 0; j < LOOPSIZE; j++) {
            for(auto i: nums1)
                a4.Find(i);
            for(auto i: nums2)
                a4.Find(i);
            for(auto i: nums3)
                a4.Find(i);
        }
        t.stop();
        cout << "all : " << t.elapsed().wall/1000000 << "ms" << endl;   //输出：start()至调用此函数的经过时间
        cout << "user : " << t.elapsed().user/1000000 << "ms" << endl;   //输出：start()至调用此函数的用户时间
        cout << "system : " << t.elapsed().system/1000000 << "ms" << endl; //输出：start()至调用此函数的系统时间
    } catch (const std::exception& e) {
        std::cout << e.what();
        return 1;
    }
    
    // cout << " 4:" << endl;
    // try {
    //     boost::timer::cpu_timer t;
    //     t.start();
    //     for(size_t j = 0; j < LOOPSIZE; j++) {
    //         for(auto i: nums1)
    //             s1.find(i);
    //         for(auto i: nums2)
    //             s1.find(i);
    //         for(auto i: nums3)
    //             s1.find(i);
    //     }
    //     t.stop();
    //     cout << "all : " << t.elapsed().wall/1000000 << "ms" << endl;   //输出：start()至调用此函数的经过时间
    //     cout << "user : " << t.elapsed().user/1000000 << "ms" << endl;   //输出：start()至调用此函数的用户时间
    //     cout << "system : " << t.elapsed().system/1000000 << "ms" << endl; //输出：start()至调用此函数的系统时间
    // } catch (const std::exception& e) {
    //     std::cout << e.what();
    //     return 1;
    // }
    // cout << " 5:" << endl;
    // try {
    //     boost::timer::cpu_timer t;
    //     t.start();
    //     for(size_t j = 0; j < LOOPSIZE; j++) {
    //         for(auto i: nums1)
    //             s2.find(i);
    //         for(auto i: nums2)
    //             s2.find(i);
    //         for(auto i: nums3)
    //             s2.find(i);
    //     }
    //     t.stop();
    //     cout << "all : " << t.elapsed().wall/1000000 << "ms" << endl;   //输出：start()至调用此函数的经过时间
    //     cout << "user : " << t.elapsed().user/1000000 << "ms" << endl;   //输出：start()至调用此函数的用户时间
    //     cout << "system : " << t.elapsed().system/1000000 << "ms" << endl; //输出：start()至调用此函数的系统时间
    // } catch (const std::exception& e) {
    //     std::cout << e.what();
    //     return 1;
    // }
    // cout << " 6:" << endl;
    // try {
    //     boost::timer::cpu_timer t;
    //     t.start();
    //     for(size_t j = 0; j < LOOPSIZE; j++) {
    //         for(auto i: nums1)
    //             s3.find(i);
    //         for(auto i: nums2)
    //             s3.find(i);
    //         for(auto i: nums3)
    //             s3.find(i);
    //     }
    //     t.stop();
    //     cout << "all : " << t.elapsed().wall/1000000 << "ms" << endl;   //输出：start()至调用此函数的经过时间
    //     cout << "user : " << t.elapsed().user/1000000 << "ms" << endl;   //输出：start()至调用此函数的用户时间
    //     cout << "system : " << t.elapsed().system/1000000 << "ms" << endl; //输出：start()至调用此函数的系统时间
    // } catch (const std::exception& e) {
    //     std::cout << e.what();
    //     return 1;
    // }
    return 0;
}
