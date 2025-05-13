// MahjongDealer.cpp 

#include "Base/BaseUtils.h"
#include "MahjongDealer.h"

#include <list>
#include <set>
#include <mutex>

#define TILE_NUMS_NO_FLOWER		136
#define TILE_NUMS_HAS_FLOWER	144

namespace NiuMa
{
	/**
	 * 麻将牌生成器，全局唯一
	 */
	class MahjongTileGenerator
	{
	public:
		MahjongTileGenerator() {
			int pat = static_cast<int>(MahjongTile::Pattern::Tong);
			unsigned int pos = 0;
			for (; pat < static_cast<int>(MahjongTile::Pattern::Dong); pat++) {
				for (int num = static_cast<int>(MahjongTile::Number::Yi);
					num <= static_cast<int>(MahjongTile::Number::Jiu); num++) {
					for (unsigned int i = 0; i < 4; i++)
						_tileBuffer[pos++] = MahjongTile::Tile(static_cast<MahjongTile::Pattern>(pat), static_cast<MahjongTile::Number>(num));
				}
			}
			for (; pat < static_cast<int>(MahjongTile::Pattern::Chun); pat++)
			{
				for (unsigned int i = 0; i < 4; i++)
					_tileBuffer[pos++] = MahjongTile::Tile(static_cast<MahjongTile::Pattern>(pat));
			}
			for (; pat <= static_cast<int>(MahjongTile::Pattern::Zhu); pat++)
				_tileBuffer[pos++] = MahjongTile::Tile(static_cast<MahjongTile::Pattern>(pat));
		}

		virtual ~MahjongTileGenerator() {}

	private:
		// 全局实例
		static std::shared_ptr<MahjongTileGenerator> s_inst;

		// 信号量
		static std::mutex s_mtx;

		// 麻将牌池
		MahjongTile::Tile _tileBuffer[TILE_NUMS_HAS_FLOWER];

	public:
		static std::shared_ptr<MahjongTileGenerator> getInstance() {
			std::lock_guard<std::mutex> lck(s_mtx);

			if (s_inst)
				return s_inst;
			s_inst = std::make_shared<MahjongTileGenerator>();
			return s_inst;
		}

		bool getTile(MahjongTile& mt) const {
			if (mt.getId() >= TILE_NUMS_HAS_FLOWER)
				return false;

			mt.setTile(_tileBuffer[mt.getId()]);
			return true;
		}

		static bool getTileById(MahjongTile& mt) {
			int id = mt.getId();
			if (id < 108) {
				int pat = 1 + id / 36;
				int num = 1 + (id % 36) / 4;
				mt.setTile(MahjongTile::Tile(static_cast<MahjongTile::Pattern>(pat), static_cast<MahjongTile::Number>(num)));
			}
			else if (id < TILE_NUMS_NO_FLOWER) {
				id -= 108;
				int pat = static_cast<int>(MahjongTile::Pattern::Dong) + id / 4;
				mt.setTile(MahjongTile::Tile(static_cast<MahjongTile::Pattern>(pat)));
			}
			else if (id < TILE_NUMS_HAS_FLOWER) {
				id -= TILE_NUMS_NO_FLOWER;
				int pat = static_cast<int>(MahjongTile::Pattern::Chun) + id;
				mt.setTile(MahjongTile::Tile(static_cast<MahjongTile::Pattern>(pat)));
			}
			else
				return false;
			return true;
		}

		static bool getIdByTile(MahjongTile& mt) {
			if (!mt.getTile().isValid())
				return false;

			int id = 0xffff;
			int pat = static_cast<int>(mt.getPattern());
			if (pat < static_cast<int>(MahjongTile::Pattern::Dong)) {
				id = (pat - 1) * 36;
				int num = static_cast<int>(mt.getNumber());
				id += (num - 1) * 4;
				mt.setId(id);
			}
			else if (pat < static_cast<int>(MahjongTile::Pattern::Chun))
				id = 108 + (pat - static_cast<int>(MahjongTile::Pattern::Dong)) * 4;
			else
				id = 136 + (pat - static_cast<int>(MahjongTile::Pattern::Chun));
			mt.setId(id);
			return true;
		}
	};

	std::shared_ptr<MahjongTileGenerator> MahjongTileGenerator::s_inst;
	std::mutex MahjongTileGenerator::s_mtx;

	MahjongDealer::MahjongDealer(bool bFlower)
		: _flower(bFlower)
		, _totalTileNums(bFlower ? TILE_NUMS_HAS_FLOWER : TILE_NUMS_NO_FLOWER)
		, _tilePool{0}
		, _start(0)
		, _end(0)
	{
		_generator = MahjongTileGenerator::getInstance();
	}

	MahjongDealer::~MahjongDealer() {}

	bool MahjongDealer::hasFlower() const {
		return _flower;
	}

	void MahjongDealer::shuffle() {
		int index = 0;
		unsigned int size = 0;
		std::list<int>::iterator it;
		std::list<int> lstIndies;
		for (int i = 0; i < _totalTileNums; i++)
			lstIndies.push_back(i);
		for (int i = 0; i < _totalTileNums; i++) {
			size = static_cast<unsigned int>(lstIndies.size());
			if (size == 1) {
				index = lstIndies.front();
				lstIndies.clear();
			} else {
				index = BaseUtils::randInt(0, static_cast<int>(size));
				it = lstIndies.begin();
				for (int j = 0; j < index; j++)
					it++;
				index = *it;
				lstIndies.erase(it);
			}
			_tilePool[i] = index;
		}
		_start = 0;
		_end = _totalTileNums;
	}

	void MahjongDealer::shuffle(const std::map<int, int>& initTiles) {
		std::set<int> setInitTiles;
		std::set<int>::const_iterator it_s;
		std::map<int, int>::const_iterator it = initTiles.begin();
		while (it != initTiles.end()) {
			int id = it->first;
			for (int i = 0; i < (it->second); i++) {
				setInitTiles.insert(id);
				id++;
			}
			++it;
		}
		int index = 0;
		unsigned int size = 0;
		unsigned int tileNums = 0;
		std::list<int>::iterator it_l;
		std::list<int> lstIndies;
		for (int i = 0; i < _totalTileNums; i++) {
			it_s = setInitTiles.find(i);
			if (it_s != setInitTiles.end())
				continue;
			lstIndies.push_back(i);
		}
		tileNums = static_cast<unsigned int>(lstIndies.size());
		for (unsigned int i = 0; i < tileNums; i++) {
			size = static_cast<unsigned int>(lstIndies.size());
			if (size == 1) {
				index = lstIndies.front();
				lstIndies.clear();
			} else {
				index = BaseUtils::randInt(0, static_cast<int>(size));
				it_l = lstIndies.begin();
				for (int j = 0; j < index; j++)
					it_l++;
				index = *it_l;
				lstIndies.erase(it_l);
			}
			_tilePool[i] = index;
		}
		_start = 0;
		_end = static_cast<int>(tileNums);
	}

	int MahjongDealer::getTotalTileNums() const {
		return _totalTileNums;
	}

	int MahjongDealer::getTileLeft() const {
		return (_end - _start);
	}

	bool MahjongDealer::isEmpty() const {
		return (_start == _end);
	}

	bool MahjongDealer::getTile(MahjongTile& mt) const {
		return _generator->getTile(mt);
	}

	bool MahjongDealer::fetchTile(MahjongTile& mt) {
		if (_start == _end)
			return false;
		mt.setId(_tilePool[_start++]);
		return _generator->getTile(mt);
	}

	bool MahjongDealer::fetchTile1(MahjongTile& mt) {
		if (_start == _end)
			return false;

		mt.setId(_tilePool[_end - 1]);
		_end--;
		return _generator->getTile(mt);
	}

	bool MahjongDealer::fetchTile(MahjongTile& mt, const std::string& str) {
		MahjongTile::Tile tile = MahjongTile::Tile::fromString(str);
		MahjongTile mt1;
		mt1.setTile(tile);
		if (!getIdByTile(mt1))
			return false;

		bool bRet = false;
		int i = _start;
		for (; i < _end; i++) {
			mt.setId(_tilePool[i]);
			if (!_generator->getTile(mt))
				continue;
			if (mt.isSame(mt1)) {
				bRet = true;
				break;
			}
		}
		if (!bRet)
			return false;
		if ((_end - i) > (i - _start + 1)) {
			while (i != _start) {
				_tilePool[i] = _tilePool[i - 1];
				i--;
			}
			_start++;
		} else {
			while ((i + 1) != _end) {
				_tilePool[i] = _tilePool[i + 1];
				i++;
			}
			_end--;
		}
		return true;
	}

	bool MahjongDealer::getTileById(MahjongTile& mt) {
		return MahjongTileGenerator::getTileById(mt);
	}

	bool MahjongDealer::getIdByTile(MahjongTile& mt) {
		return MahjongTileGenerator::getIdByTile(mt);
	}
}