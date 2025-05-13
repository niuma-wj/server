// MahjongTile.cpp

#include "MahjongTile.h"

#include <map>

namespace NiuMa
{
	const int MahjongTile::INVALID_ID = -1;
	const char* MahjongTile::PATTERN_NAMES[18] = { "筒", "条", "万", "东", "南", "西", "北", "中", "发", "白", "春", "夏", "秋", "冬", "梅", "兰", "菊", "竹" };
	const char* MahjongTile::NUMBER_NAMES[9] = { "一", "二", "三", "四", "五", "六", "七", "八", "九" };

	MahjongTile::Tile::Tile()
		: pattern(static_cast<int>(Pattern::Invalid))
		, number(static_cast<int>(Number::Invalid))
	{}

	MahjongTile::Tile::Tile(const Tile& t)
		: pattern(static_cast<int>(t.getPattern()))
		, number(static_cast<int>(t.getNumber()))
	{}

	MahjongTile::Tile::Tile(MahjongTile::Pattern p, MahjongTile::Number n)
		: pattern(static_cast<int>(p))
		, number(static_cast<int>(n))
	{}

	MahjongTile::Tile& MahjongTile::Tile::operator=(const Tile& t) {
		pattern = static_cast<int>(t.getPattern());
		number = static_cast<int>(t.getNumber());
		return *this;
	}
	
	bool MahjongTile::Tile::operator==(const Tile& t) const {
		if (pattern != static_cast<int>(t.getPattern()))
			return false;
		if (number != static_cast<int>(t.getNumber()))
			return false;
		return true;
	}

	bool MahjongTile::Tile::operator!=(const Tile& t) const {
		if (pattern != static_cast<int>(t.getPattern()))
			return true;
		if (number != static_cast<int>(t.getNumber()))
			return true;

		return false;
	}

	bool MahjongTile::Tile::operator>(const Tile& t) const {
		if (pattern > static_cast<int>(t.getPattern()))
			return true;
		else if (pattern < static_cast<int>(t.getPattern()))
			return false;
		if (number > static_cast<int>(t.getNumber()))
			return true;

		return false;
	}

	bool MahjongTile::Tile::operator>=(const Tile& t) const {
		if (pattern > static_cast<int>(t.getPattern()))
			return true;
		else if (pattern < static_cast<int>(t.getPattern()))
			return false;
		if (number < static_cast<int>(t.getNumber()))
			return false;
		return true;
	}

	bool MahjongTile::Tile::operator<(const Tile& t) const {
		if (pattern > static_cast<int>(t.getPattern()))
			return false;
		else if (pattern < static_cast<int>(t.getPattern()))
			return true;
		if (number < static_cast<int>(t.getNumber()))
			return true;
		return false;
	}

	bool MahjongTile::Tile::operator<=(const Tile& t) const {
		if (pattern > static_cast<int>(t.getPattern()))
			return false;
		else if (pattern < static_cast<int>(t.getPattern()))
			return true;
		if (number > static_cast<int>(t.getNumber()))
			return false;
		return true;
	}

	MahjongTile::Pattern MahjongTile::Tile::getPattern() const {
		return static_cast<Pattern>(pattern);
	}

	MahjongTile::Number MahjongTile::Tile::getNumber() const {
		return static_cast<Number>(number);
	}

	bool MahjongTile::Tile::isValid() const {
		if (pattern == static_cast<int>(Pattern::Invalid))
			return false;
		if (isNumbered()) {
			if (number == static_cast<int>(Number::Invalid))
				return false;
		}
		else if (number != static_cast<int>(Number::Invalid))
			return false;

		return true;
	}

	void MahjongTile::Tile::setInvalid() {
		pattern = static_cast<int>(Pattern::Invalid);
		number = static_cast<int>(Number::Invalid);
	}

	bool MahjongTile::Tile::isNumbered() const {
		if (static_cast<int>(Pattern::Tong) == pattern ||
			static_cast<int>(Pattern::Tiao) == pattern ||
			static_cast<int>(Pattern::Wan) == pattern)
			return true;

		return false;
	}

	bool MahjongTile::Tile::isAdjacent(const Tile& t, bool astride) const {
		if (pattern != static_cast<int>(t.getPattern()))
			return false;
		if (!isNumbered())
			return false;
		if ((static_cast<int>(Number::Invalid) == number) || (Number::Invalid == t.getNumber()))
			return false;
		int n1 = number;
		int n2 = static_cast<int>(t.getNumber());
		n1 -= n2;
		if (astride) {
			if (n1 == 2 || n1 == -2)
				return true;
		}
		else if (n1 == 1 || n1 == -1)
			return true;

		return false;
	}

	void MahjongTile::Tile::toString(std::string& text) const {
		text.clear();
		if (isNumbered() && (number < 10) && (number > 0))
			text = NUMBER_NAMES[number - 1];
		if (pattern < 19 && pattern > 0)
			text = text + PATTERN_NAMES[pattern -1];
	}

	MahjongTile::Tile MahjongTile::Tile::fromString(const std::string& text) {
		static bool s_bFirst = true;
		static std::map<std::string, Tile> s_mapTiles;
		if (s_bFirst) {
			s_bFirst = false;
			s_mapTiles.insert(std::make_pair(std::string("一筒"), Tile(Pattern::Tong, Number::Yi)));
			s_mapTiles.insert(std::make_pair(std::string("二筒"), Tile(Pattern::Tong, Number::Er)));
			s_mapTiles.insert(std::make_pair(std::string("三筒"), Tile(Pattern::Tong, Number::San)));
			s_mapTiles.insert(std::make_pair(std::string("四筒"), Tile(Pattern::Tong, Number::Si)));
			s_mapTiles.insert(std::make_pair(std::string("五筒"), Tile(Pattern::Tong, Number::Wu)));
			s_mapTiles.insert(std::make_pair(std::string("六筒"), Tile(Pattern::Tong, Number::Liu)));
			s_mapTiles.insert(std::make_pair(std::string("七筒"), Tile(Pattern::Tong, Number::Qi)));
			s_mapTiles.insert(std::make_pair(std::string("八筒"), Tile(Pattern::Tong, Number::Ba)));
			s_mapTiles.insert(std::make_pair(std::string("九筒"), Tile(Pattern::Tong, Number::Jiu)));
			s_mapTiles.insert(std::make_pair(std::string("一条"), Tile(Pattern::Tiao, Number::Yi)));
			s_mapTiles.insert(std::make_pair(std::string("二条"), Tile(Pattern::Tiao, Number::Er)));
			s_mapTiles.insert(std::make_pair(std::string("三条"), Tile(Pattern::Tiao, Number::San)));
			s_mapTiles.insert(std::make_pair(std::string("四条"), Tile(Pattern::Tiao, Number::Si)));
			s_mapTiles.insert(std::make_pair(std::string("五条"), Tile(Pattern::Tiao, Number::Wu)));
			s_mapTiles.insert(std::make_pair(std::string("六条"), Tile(Pattern::Tiao, Number::Liu)));
			s_mapTiles.insert(std::make_pair(std::string("七条"), Tile(Pattern::Tiao, Number::Qi)));
			s_mapTiles.insert(std::make_pair(std::string("八条"), Tile(Pattern::Tiao, Number::Ba)));
			s_mapTiles.insert(std::make_pair(std::string("九条"), Tile(Pattern::Tiao, Number::Jiu)));
			s_mapTiles.insert(std::make_pair(std::string("一万"), Tile(Pattern::Wan, Number::Yi)));
			s_mapTiles.insert(std::make_pair(std::string("二万"), Tile(Pattern::Wan, Number::Er)));
			s_mapTiles.insert(std::make_pair(std::string("三万"), Tile(Pattern::Wan, Number::San)));
			s_mapTiles.insert(std::make_pair(std::string("四万"), Tile(Pattern::Wan, Number::Si)));
			s_mapTiles.insert(std::make_pair(std::string("五万"), Tile(Pattern::Wan, Number::Wu)));
			s_mapTiles.insert(std::make_pair(std::string("六万"), Tile(Pattern::Wan, Number::Liu)));
			s_mapTiles.insert(std::make_pair(std::string("七万"), Tile(Pattern::Wan, Number::Qi)));
			s_mapTiles.insert(std::make_pair(std::string("八万"), Tile(Pattern::Wan, Number::Ba)));
			s_mapTiles.insert(std::make_pair(std::string("九万"), Tile(Pattern::Wan, Number::Jiu)));
			s_mapTiles.insert(std::make_pair(std::string("东"), Tile(Pattern::Dong, Number::Invalid)));
			s_mapTiles.insert(std::make_pair(std::string("南"), Tile(Pattern::Nan, Number::Invalid)));
			s_mapTiles.insert(std::make_pair(std::string("西"), Tile(Pattern::Xi, Number::Invalid)));
			s_mapTiles.insert(std::make_pair(std::string("北"), Tile(Pattern::Bei, Number::Invalid)));
			s_mapTiles.insert(std::make_pair(std::string("中"), Tile(Pattern::Zhong, Number::Invalid)));
			s_mapTiles.insert(std::make_pair(std::string("发"), Tile(Pattern::Fa, Number::Invalid)));
			s_mapTiles.insert(std::make_pair(std::string("白"), Tile(Pattern::Bai, Number::Invalid)));
			s_mapTiles.insert(std::make_pair(std::string("春"), Tile(Pattern::Chun, Number::Invalid)));
			s_mapTiles.insert(std::make_pair(std::string("夏"), Tile(Pattern::Xia, Number::Invalid)));
			s_mapTiles.insert(std::make_pair(std::string("秋"), Tile(Pattern::Qiu, Number::Invalid)));
			s_mapTiles.insert(std::make_pair(std::string("冬"), Tile(Pattern::Winter, Number::Invalid)));
			s_mapTiles.insert(std::make_pair(std::string("梅"), Tile(Pattern::Mei, Number::Invalid)));
			s_mapTiles.insert(std::make_pair(std::string("兰"), Tile(Pattern::Lan, Number::Invalid)));
			s_mapTiles.insert(std::make_pair(std::string("菊"), Tile(Pattern::Ju, Number::Invalid)));
			s_mapTiles.insert(std::make_pair(std::string("竹"), Tile(Pattern::Zhu, Number::Invalid)));
		}
		std::map<std::string, Tile>::const_iterator it = s_mapTiles.find(text);
		if (it != s_mapTiles.end())
			return it->second;

		return Tile();
	}

	MahjongTile::MahjongTile()
		: id(INVALID_ID)
	{}

	MahjongTile::MahjongTile(const MahjongTile& mt)
		: id(mt.getId())
		, tile(static_cast<Pattern>(mt.getPattern()), static_cast<Number>(mt.getNumber()))
	{}

	MahjongTile::MahjongTile(int id, Pattern p, Number n)
		: id(id)
		, tile(p, n)
	{}
	
	MahjongTile::~MahjongTile()
	{}

	MahjongTile& MahjongTile::operator=(const MahjongTile& mt) {
		tile = mt.getTile();
		id = mt.getId();

		return *this;
	}

	bool MahjongTile::operator==(const MahjongTile& mt) const {
		return (id == mt.getId());
	}

	bool MahjongTile::operator!=(const MahjongTile& mt) const {
		return (id != mt.getId());
	}

	bool MahjongTile::operator>(const MahjongTile& mt) const {
		return (id > mt.getId());
	}

	bool MahjongTile::operator>=(const MahjongTile& mt) const {
		return (id >= mt.getId());
	}

	bool MahjongTile::operator<(const MahjongTile& mt) const {
		return (id < mt.getId());
	}

	bool MahjongTile::operator<=(const MahjongTile& mt) const {
		return (id <= mt.getId());
	}

	bool MahjongTile::isValid() const {
		if (id == INVALID_ID)
			return false;
		return tile.isValid();
	}

	void MahjongTile::setInvalid() {
		id = INVALID_ID;
		tile.setInvalid();
	}

	void MahjongTile::setId(int id_) {
		id = id_;
	}

	int MahjongTile::getId() const {
		return id;
	}

	void MahjongTile::setTile(const Tile& t) {
		tile = t;
	}

	const MahjongTile::Tile& MahjongTile::getTile() const {
		return tile;
	}

	MahjongTile::Pattern MahjongTile::getPattern() const {
		return tile.getPattern();
	}

	MahjongTile::Number MahjongTile::getNumber() const {
		return tile.getNumber();
	}

	bool MahjongTile::isSame(const MahjongTile& mt) const {
		return (tile == mt.getTile());
	}

	bool MahjongTile::isSame(const MahjongTile::Tile& t) const {
		return (tile == t);
	}

	bool MahjongTile::isAdjacent(const MahjongTile::Tile& t, bool astride) const {
		return tile.isAdjacent(t, astride);
	}
}