#include "LRU_cache.hpp"

using namespace std;

string val_to_bin(u32 __v, u32 __SZ){
    string ans = "";
    if (!__v) { return make_sz("0", __SZ); }
    while(__v > 0) {
	    ans += char(__v % 2 + 48);
	    __v/=2;
    }
    reverse(all(ans));
    return make_sz(ans, __SZ);
}

string make_sz(string __s, u32 __SZ) {
    string ans = "";
    REP(_z, 0, (int)(__SZ - __s.length())) ans += '0';
    ans += __s;
    return ans;
}
set_associative::set_associative(int __asc, int c_l, int blk_SZ)
{
    this->assoc = __asc;
    this->cache_line = c_l;
    this->blk_SZ = blk_SZ;
    data.resize(c_l / __asc, vector<vector<u32>> (__asc, vector<u32> (blk_SZ)));
	count.resize(c_l, 0);
	empty.resize(c_l, true);
	tag.resize(c_l);
}

void set_associative::__incre__(){
    REP(_z, 0, cache_line){
	    if (!empty[_z]) {
		    if (count[_z] == MA) continue;
		    count[_z]++;
	    }
	}
}  

optional<u32> set_associative::cache_read(u32 addr, u32 set_addr, u32 off) {
	__incre__();
	REP(_z, 0, assoc) {
	    if (!empty[set_addr * assoc + _z] && tag[set_addr * assoc + _z] == addr) {
		count[set_addr * assoc + _z] = 0;
		return data[set_addr][_z][off];
	    }
	}
	return -1;
}

void set_associative::cache_write(u32 addr, u32 set_addr, u32 off, u32 _val) {
	__incre__();
	REP(_z, 0, assoc){
	    if (!empty[set_addr * assoc + _z] && tag[set_addr * assoc + _z] == addr) {
		    data[set_addr][_z][off] = _val;
		    count[set_addr * assoc + _z] = 0;
		    return;
	    }
	}
	
	int u_lim {0}, idx {-1};
	REP(_z, 0, assoc) {
	    // [set_addr * assoc + iter] MAP (iter) ELEMS OFF set_addr-th SET TO ITS IDX IN empty, tag, count ARR
	    if (empty[set_addr * assoc + _z]) {
		    count[set_addr * assoc + _z] = 0;
		    tag[set_addr * assoc + _z] = addr;
		    empty[set_addr * assoc + _z] = false;
		    REP(_j, 0, blk_SZ){
		        if (_j == off) { data[set_addr][_z][_j] = _val; }
                else{ data[set_addr][_z][_j] = 0; }
		    }
		    return;
	    }else {
		if (count[set_addr * assoc + _z] > u_lim) {
		        u_lim = count[set_addr * assoc + _z];
		        idx = _z;
		    }
	    }
	}
	count[set_addr * assoc + idx] = 0;
	tag[set_addr * assoc + idx] = addr;
	REP(_z, 0, blk_SZ) {
	    if (_z == off) { data[set_addr][idx][_z] = _val; }
        else { data[set_addr][idx][_z] = 0; }
    }
}
    
void set_associative::__print__() {
	REP(_z, 0, cache_line) {
	    if (empty[_z]) {
		    cout << "##EMPTY##" << endl;
		    continue;
	    }
	
	    if (_z % assoc == 0) { cout << endl; }
		// differentiate between sets
	    
	    cout << "##BLOCK## " << val_to_bin(tag[_z], 32 - __lg(blk_SZ) - __lg(cache_line / assoc)) << " ##DATA## : ";
	    
	    REP(_j, 0, blk_SZ) { cout << data[_z / assoc][_z % assoc][_j] << " "; }
	    cout << endl;
	}
}


/**

//       cache__.resize(4, list<CACHE_blk>(assoc));
//      @param : (SET_NUM, list<CACHE_blk>(assoc));
//                  set_idx = addr % SET_NUM

optional<u32> set_associative::read(u32 addr) {
    int set_idx = addr / 4;
    u32 tag = addr / 4;
    auto it = tag_cache_blk_map.find(tag);
    if (it != tag_cache_blk_map.end() && it->second->valid) {
        // CACHE_HIT => UPDATE LRU
        cache__[idx].splice(cache__[idx].begin(), cache__[idx], it->second);
        return it->second->datx_[addr % 2]; //
    }
    return std::nullopt; // Cache miss
}
    
void set_associative::write(u32 addr, u32 data) {
    int idx = set_idx(addr);
    u64 tag = addr / 4;

    auto it = tag_cache_blk_map.find(tag);
    if (it != tag_cache_blk_map.end()) {
        // If tag exists, update the block and move to the front (LRU)
        it->second->datx_[addr % 8] = data;
        cache__[idx].splice(cache__[idx].begin(), cache__[idx], it->second);
    } else {
        // If tag doesn't exist, insert a new block and check for eviction
        CACHE_blk new_blk;
        new_blk.valid = true;
        new_blk.__tag = tag;
        new_blk.datx_[addr % 8] = data;
        cache__[idx].push_front(new_blk);
        tag_cache_blk_map[tag] = cache__[idx].begin();

        if (cache__[idx].size() > 2) {
            tag_cache_blk_map.erase(cache__[idx].back().__tag);
            cache__[idx].pop_back();
        }
    }
}

*/