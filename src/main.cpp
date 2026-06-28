#include <vector>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <fstream>

int alloc = 0, dealloc = 0;

/**
 * Structure that simulates a page in a file. 
 * 
 * It is created with a fixed size (utilizing vectors `reserve` method)
 * Pages also posses pointers to the next and previous page, being structured like a double linked list
 * This structure allows for adding elements in the end with only one access
 * The double linked nature makes it also easier to implement iterators and getting an end iterator in O(1)
 */
struct Page {
    using PagePtr = Page*;
    struct iterator {
        iterator(PagePtr ptr, size_t ind): m_page(ptr), m_index(ind) { }
        int &operator*() { return m_page->m_data[m_index]; }
        iterator &operator++() {
            if(m_index == m_page->m_data.capacity()-1) {
                m_page = m_page->m_next;
                m_index = 0;
            }else
                ++m_index;
            return *this;
        }
        iterator operator++(int) {
            auto ret = *this;
            if(m_index == m_page->m_data.capacity()-1) {
                m_page = m_page->m_next;
                m_index = 0;
            }else
                ++m_index;
            return ret;
        }
        bool operator==(const iterator &it) { return m_page == it.m_page && m_index == it.m_index; }
        bool operator!=(const iterator &it) { return !operator==(it); }
        operator bool() const { return m_index != m_page->m_data.size(); }
        bool isPastEnd() const { return m_index == m_page->m_data.capacity(); }
        PagePtr page() { return m_page; }
        size_t index() { return m_index; }
    protected:
        PagePtr m_page;
        size_t m_index;
    };

    Page(size_t n) {
        m_data.reserve(n);
        m_next = m_prev = this;
        ++alloc;
    }

    ~Page() { ++dealloc; }

    /** Search for the element `x` in the page and increase `count` for each new page visited
     * 
     * @returns and iterator pointing to the location of `x` in the linked pages if found or end() otherwise
     */
    iterator get(int x, size_t &count) {
        iterator it = begin();
        int cnt = -1*(it!=end());
        for(; cnt ; cnt+=it==back(), ++it) {
            count += it.index()==0;
            if(*it == x)
                return it;
        } 
        return end();
    }

    /**insert the element `x` in the end of the pages list.
     * 
     * If the final page is full, creates a new page at the end of the list.
     */
    iterator push(int x) {
        if(end().isPastEnd()) {
            PagePtr ptr = new Page(m_data.capacity());
            ptr->m_prev = m_prev;
            ptr->m_next = this;
            m_prev->m_next = ptr;
            m_prev = ptr;
        }
        m_prev->m_data.emplace_back(x);
        return back();
    }

    /**Erases all elements in range [left, right]
     * 
     * Empty pages are deleted with the only excpetion being the head page.
    */
    size_t erase(iterator left, iterator right) {
        size_t erased = 0;
        while(left.page() != right.page()) {
            PagePtr prev = right.page()->m_prev;
            prev->m_next = right.page()->m_next;
            prev->m_next->m_prev = prev->m_next;
            delete right.page();
            right = {prev,0};
            ++erased;
        }
        if(left.page() != this && left.index() == 0) {
            left.page()->m_next->m_prev = left.page()->m_prev;
            left.page()->m_prev->m_next = left.page()->m_next;
            delete left.page();
            ++erased;
        }else {
            left.page()->m_data.resize(left.index());
        }
        return erased;
    }

    size_t size() { return m_data.size(); }
    iterator begin() { return {this, 0}; }
    iterator end() { return {m_prev, m_prev->m_data.size()}; }
    iterator back() { return {m_prev, m_prev->m_data.size()-1}; }
    PagePtr last() { return m_prev; }
private:
    std::vector<int> m_data;
    PagePtr m_next, m_prev;
};


/** Hash Table container implemented with the linear hashing algorithm.
 * 
 * The container grows linearly along with the input size, allocating memory only when needed.
 * The hash function and the table are built in levels, starting at level 0.
 * A parameter `alfa` stipulates the maximum percentual of occupation of the table. 
 * When this value maximum is violated, new pages are created and the table is redistributed.
 */
class LinearHash {
public:
    using PageIt = Page::iterator;
    using PagePtr = Page*;

    /** Hash Table that grows linearly with the input size
     * 
     * @param n initial size of the table, i.e the initial number of pages
     * @param page the fixed size of the pages
     * @param alfa the maximum occupation percentual in the table
     */
    LinearHash(size_t n = 1, size_t page = 10, float alfa = 0.75)
    : m_capacity(n), m_initialCapacity(n), m_pageSize(page), m_alfa(alfa) 
    { 
        m_pages.reserve(n);
        for(size_t i = 0; i < n; ++i)
            m_pages.emplace_back(new Page(m_pageSize));
    }

    /** Inserts element `x` in the table if not present and return its iterator
     * 
     * @return An iterator to the position of element x in the table. May be old or just added.
     */
    PageIt insert(int x) {
        m_diskAccess = 0;
        size_t index = hash(x);
        if(index < m_nextPage)
            index = nextHash(x);
        PageIt it = m_pages[index]->get(x, m_diskAccess);
        if(it)
            return it; // Already in the table
        it = m_pages[index]->push(x); // inserts it in the page
        checkUpdate(index);
        bool moved = resize(index);
        if(moved) {
            if(index < m_nextPage) // This check is neccessary in case of level upping
                index = nextHash(x);
            else
                index = hash(x);
            return m_pages[index]->back();
        }
        return it;
    }

    /** Searches for element `x` in the table and returns its iterator
     * 
     * @return an iterator for the position of element `x` or its pages `end()` iterator
     */
    PageIt search(int x) {
        m_diskAccess = 0;
        size_t index = hash(x);
        if(index < m_nextPage)
            index = nextHash(x);
        return m_pages[index]->get(x, m_diskAccess);
    }

    /** Calculates the current alfa, i.e the current percentual of occupation of the table
     * 
     * @return alfa = size() / ((capacity() + overflow()) * pageSize)
     */
    float currentAlfa() const {
        return static_cast<float>(m_size) / static_cast<float>((m_capacity + m_extraPages) * m_pageSize); 
    }
    size_t capacity() const { return m_capacity; }
    size_t overflow() const { return m_extraPages; }
    size_t diskAccess() const { return m_diskAccess; }
    
    ~LinearHash() {
        while(m_pages.size()) {
            delete m_pages.back();
            m_pages.pop_back();
        }
    }
private:

    /* Updates the quantities of pages after an insertion */
    void checkUpdate(size_t index) {
        PagePtr last = m_pages[index]->last();
        if(last != m_pages[index] && last->size() == 1) {
            ++m_extraPages; // Overflow page just created
        }
        ++m_size;
    }    

    /* Checks if the alfaMax has been violated and resizes the table */
    bool resize(size_t index) {
        bool movedIndex = false;
        while(currentAlfa() > m_alfa) {
            if(m_nextPage == index)
                movedIndex = true; // To know if the element may have been reallocated

            m_pages.emplace_back(new Page(m_pageSize));
            ++m_capacity;

            PageIt lit = m_pages[m_nextPage]->begin(), rit = lit;
            int cnt = -1*(lit!=m_pages[m_nextPage]->end());
            // Reinsert all elements in the page
            // Using two pointer thecnique to avoid unnecessary duplication/reallocation
            // lis is an iterator poiting to the next position of the old page that will receive a staying element
            // rit is the next element to be reinserted
            for(; cnt ; cnt+=rit==m_pages[m_nextPage]->back(), ++rit) {
                size_t index = nextHash(*rit);
                if(index == m_nextPage) {
                    // If the element will continue in the page, put it in lit and advance lit
                    std::swap(*lit, *rit);
                    ++lit;
                }else {
                    // Else, move it to the new page
                    PageIt it = m_pages[index]->push(std::move(*rit));
                    m_extraPages += it.page() != m_pages[index] && it.index()==0; 
                }
            }
            // Erases the pages that aren't needed anymore
            m_extraPages -= m_pages[m_nextPage]->erase(lit, m_pages[m_nextPage]->end());
            ++m_nextPage;
            // Checks if its time to go to the next level and reset m_nextPage (N)
            if(m_nextPage == m_initialCapacity * (1 << m_level)) {
                ++m_level;
                m_nextPage = 0;
            }
        }
        return movedIndex;
    }

    /* Hash of the current level */
    size_t hash(int x) const {
        return x % (m_initialCapacity * (1<<m_level));
    }

    /* Hash of the next level */
    size_t nextHash(int x) const {
        return x % (m_initialCapacity * (1<<(m_level+1)));
    }

    /** In order:
     * 
     * `m_size`: count of elements currently in the table
     * `m_initialCapacity`: how many pages were allocated initially
     * `m_capacity`: How many pages lists are allocated (size of m_pages)
     * `m_extraPages`: How many extra pages are allocated because of overflow
     * `m_pageSize`: Fixed size of a page 
     * `m_level`: current level of the table (for the hash function)
     * `m_nextPage`: Next page to be taken to a further level
     * `m_diskAccess`: internal variable to account how many pages are accessed in a read operation
     * `m_alfa`: Maximum occupation percentual
     * `m_pages`: Vector of pages
     */
    size_t m_size = 0;
    size_t m_initialCapacity;
    size_t m_capacity;
    size_t m_extraPages = 0;
    size_t m_pageSize;
    size_t m_level = 0;
    size_t m_nextPage = 0;
    size_t m_diskAccess = 0;
    float m_alfa;
    std::vector<PagePtr> m_pages;
};

int main(int argc, char **argv) {
    int p = 10;
    if(argc>1)
        p = std::stoi(argv[1]);
    float alfaMax = 0.75;
    if(argc>2)
        alfaMax = std::stof(argv[2]);
    LinearHash lh(1,p,alfaMax);
    int n; std::cin >> n;
    // Insert all elements
    for(int i = 0; i < n; ++i) {
        int x; std::cin >> x;
        lh.insert(x);
    }
    float alfa = lh.currentAlfa();
    size_t capacity = lh.capacity();
    size_t overflow = lh.overflow();
    size_t diskAccessIn = 0;
    size_t diskAccessOut = 0;
    int q1; std::cin >> q1;
    // First search (contained elements)
    for(int i = 0; i < q1; ++i) {
        int x; std::cin >> x;
        lh.search(x);
        diskAccessIn += lh.diskAccess();
    }
    int q2; std::cin >> q2;
    // Second search (non-contained elements)
    for(int i = 0; i < q2; ++i) {
        int x; std::cin >> x;
        lh.search(x);
        diskAccessOut += lh.diskAccess();
    }

    std::string header = "n;searchIn;searchOut;pageSize;alfaMax;capacity;oveflow;alfa;diskAccessIn;diskAccessOut;input";
    if(argc>3) {
        namespace fs = std::filesystem;
        std::string csvPathStr = argv[3];
        fs::path csvPath(csvPathStr);
        if(!fs::is_directory(csvPath.parent_path()))
            fs::create_directories(csvPath.parent_path());
        if(!fs::is_regular_file(csvPath)) {
            std::ofstream out(csvPath);
            out << header << std::endl;
            out.close();
        }
        std::ofstream out(csvPath, std::ios::app);
        out << std::setprecision(2) << std::fixed;
        out 
            << n << ";" << q1 << ";" << q2 << ";" << p << ";" 
            << alfaMax << ";" << capacity << ";" << overflow << ";"
            << alfa << ";" << diskAccessIn << ";" << diskAccessOut; 
        if(argc>4)
            out << ";" << argv[4];
        out << std::endl;
    }else {
        std::cout << header << std::endl;
        std::cout << std::setprecision(2) << std::fixed;
        std::cout 
            << n << "\t;" << q1 << "\t\t;" << q2 << "\t\t;" << p << "\t\t;" 
            << alfaMax << "\t\t;" << capacity << "\t\t;" << overflow << "\t\t;"
            << alfa << "\t;" << diskAccessIn << "\t\t;" << diskAccessOut << std::endl; 
    }
    return 0;
}