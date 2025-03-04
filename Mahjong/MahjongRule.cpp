// MahjongRule.cpp

#include "MahjongRule.h"

#include "Base/Log.h"

namespace NiuMa
{
	MahjongRule::MahjongRule()
		: _13Lan(false)
	{}

	MahjongRule::~MahjongRule()
	{}

	bool MahjongRule::hasFlower() const {
		return false;
	}

	void MahjongRule::set13Lan(bool s) {
		_13Lan = s;
	}

	bool MahjongRule::is13Lan() const {
		return _13Lan;
	}

	void MahjongRule::checkTingPai(const MahjongTileArray& tiles, const MahjongTile::TileArray& allGotTiles, MahjongGenre::TingPaiArray& tps, MahjongAvatar* pAvatar) const {
		tps.clear();
		unsigned int nums = static_cast<unsigned int>(tiles.size());
		if (nums > 13)	// 超过13张牌，大相公无法胡牌
			return;
		bool bTest = false;
		// 七小对听牌的同时也可能平胡听牌，如：1122334455667，听7：11 22 33 44 55 66 77， 听1：11 123 234 456 567
		PingHuStyle ps;
		if (checkPingHu(tiles, tps, &ps))
			bTest = true;
		if (checkQiXiaoDui(tiles, tps))
			bTest = true;
		if (!bTest && checkShiSanYao(tiles, tps))
			bTest = true;
		if (!bTest && _13Lan && checkShiSanLan(tiles, tps))
			bTest = true;
		if (checkHuStyleEx(tiles, tps, pAvatar))
			bTest = true;
		if (!bTest)
			return;
		bTest = false;
		nums = static_cast<unsigned int>(tps.size());
		MahjongTile::TileArray::const_iterator it_;
		MahjongGenre::TingPaiArray::const_iterator it = tps.begin();
		while (it != tps.end()) {
			it_ = allGotTiles.begin();
			while (it_ != allGotTiles.end()) {
				if (*it_ == it->tile) {
					bTest = true;
					break;
				}
				++it_;
			}
			if (bTest) {
				// 所听的牌4张都被自己拿了(例如碰了3张，手上还有1张，或者杠了4张)，则不能听这张牌
				bTest = false;
				it = tps.erase(it);
			}
			else
				++it;
		}
		if (nums == 1 && !tps.empty()) {
			// 仅听一张牌，可能是单吊、边张、卡张
			if (tps[0].style == static_cast<int>(MahjongGenre::HuStyle::PingHu)) {
				if (ps.style != 0)
					tps[0].style = ps.style;
				else {
					// 在平胡仅听一张牌的时候，必定是单吊或边张或卡张，否则说明算法有误，打出log
					std::string strLog = "单吊-边张-卡张检测有误，手牌:";
					std::string strTmp;
					getTileArrayString(tiles, strTmp);
					strLog += strTmp;
					LOG_ERROR(strLog);
				}
			}
		}
	}

	bool MahjongRule::checkShiSanYao(const MahjongTileArray& tiles, MahjongGenre::TingPaiArray& tps) const {
		if (tiles.size() != 13)
			return false;

		// 十三幺：一筒、九筒、一条、九条、一万、九万、东、南、西、北、中、发、白
		static const MahjongTile::Tile SSY_TILE_ARRAY[] = {
			MahjongTile::Tile(MahjongTile::Pattern::Tong, MahjongTile::Number::Yi),
			MahjongTile::Tile(MahjongTile::Pattern::Tong, MahjongTile::Number::Jiu),
			MahjongTile::Tile(MahjongTile::Pattern::Tiao, MahjongTile::Number::Yi),
			MahjongTile::Tile(MahjongTile::Pattern::Tiao, MahjongTile::Number::Jiu),
			MahjongTile::Tile(MahjongTile::Pattern::Wan, MahjongTile::Number::Yi),
			MahjongTile::Tile(MahjongTile::Pattern::Wan, MahjongTile::Number::Jiu),
			MahjongTile::Tile(MahjongTile::Pattern::Dong),
			MahjongTile::Tile(MahjongTile::Pattern::Nan),
			MahjongTile::Tile(MahjongTile::Pattern::Xi),
			MahjongTile::Tile(MahjongTile::Pattern::Bei),
			MahjongTile::Tile(MahjongTile::Pattern::Zhong),
			MahjongTile::Tile(MahjongTile::Pattern::Fa),
			MahjongTile::Tile(MahjongTile::Pattern::Bai)
		};
		for (unsigned int i = 0; i < 13; i++) {
			if (!tiles[i].isSame(SSY_TILE_ARRAY[i]))
				return false;
		}
		MahjongGenre::TingPai tp;
		tp.style = static_cast<int>(MahjongGenre::HuStyle::ShiSanYao);
		for (unsigned int i = 0; i < 13; i++) {
			tp.tile = SSY_TILE_ARRAY[i];
			tps.push_back(tp);
		}
		return true;
	}

	bool checkShiSanLanN(const MahjongTileArray& tiles, MahjongTile::Pattern p, int& nums, MahjongTile arrTiles[3]) {
		nums = 0;
		MahjongTileArray::const_iterator it = tiles.begin();
		while (it != tiles.end()) {
			const MahjongTile& mt = *it;
			if (mt.getPattern() == p) {
				if (nums > 2)
					return false;
				arrTiles[nums++] = mt;
			}
			else if (mt.getPattern() > p)
				break;

			++it;
		}
		if (nums < 2)
			return true;
		int n1 = 0;
		int n2 = 0;
		if (nums > 1) {
			n1 = static_cast<int>(arrTiles[0].getNumber());
			n2 = static_cast<int>(arrTiles[1].getNumber());
			if (n2 - n1 < 3)
				return false;
		}
		if (nums > 2) {
			n1 = static_cast<int>(arrTiles[1].getNumber());
			n2 = static_cast<int>(arrTiles[2].getNumber());
			if (n2 - n1 < 3)
				return false;
		}
		return true;
	}

	void FindShiSanLan(MahjongTile::Pattern p, int nums, MahjongTile arrTiles[3], MahjongTile::TileArray& arrAdds) {
		if (nums > 2)
			return;

		static const MahjongTile::Number NUMBER_ARRAY[] = {
			MahjongTile::Number::Yi,
			MahjongTile::Number::Er,
			MahjongTile::Number::San,
			MahjongTile::Number::Si,
			MahjongTile::Number::Wu,
			MahjongTile::Number::Liu,
			MahjongTile::Number::Qi,
			MahjongTile::Number::Ba,
			MahjongTile::Number::Jiu
		};
		int n0 = 0;
		int n1 = 0;
		int n2 = 0;
		bool bTest = true;
		for (unsigned int i = 0; i < 9; i++) {
			n1 = static_cast<int>(NUMBER_ARRAY[i]);
			for (unsigned char j = 0; j < nums; j++) {
				n2 = static_cast<int>(arrTiles[j].getNumber());
				n0 = n1 - n2;
				if (n0 < 3 && n0 > -3) {
					bTest = false;
					break;
				}
			}
			if (bTest)
				arrAdds.push_back(MahjongTile::Tile(p, NUMBER_ARRAY[i]));
			else
				bTest = true;
		}
	}

	bool MahjongRule::checkShiSanLan(const MahjongTileArray& tiles, MahjongGenre::TingPaiArray& tps) const {
		if (tiles.size() != 13)
			return false;

		// 中南西北中发白七个汉字的数量
		int index = 0;
		int arrZiNums[7] = { 0 };
		MahjongTile::Pattern ePat = MahjongTile::Pattern::Invalid;
		for (unsigned int i = 0; i < 13; i++) {
			ePat = tiles[i].getPattern();
			if (ePat < MahjongTile::Pattern::Dong || ePat > MahjongTile::Pattern::Bai)
				continue;

			index = static_cast<int>(ePat) - static_cast<int>(MahjongTile::Pattern::Dong);
			arrZiNums[index] += 1;
		}
		int ziNums = 0;
		int tongNums = 0;
		int tiaoNums = 0;
		int wanNums = 0;
		for (unsigned int i = 0; i < 7; ++i) {
			if (arrZiNums[i] > 1)
				return false;
			ziNums += arrZiNums[i];
		}
		if (ziNums < 4)
			return false;
		MahjongTile arrTongTiles[3];
		MahjongTile arrTiaoTiles[3];
		MahjongTile arrWanTiles[3];
		if (!checkShiSanLanN(tiles, MahjongTile::Pattern::Tong, tongNums, arrTongTiles))
			return false;
		if (!checkShiSanLanN(tiles, MahjongTile::Pattern::Tiao, tiaoNums, arrTiaoTiles))
			return false;
		if (!checkShiSanLanN(tiles, MahjongTile::Pattern::Wan, wanNums, arrWanTiles))
			return false;

		MahjongGenre::TingPai tp;
		if (ziNums < 6)
			tp.style = static_cast<int>(MahjongGenre::HuStyle::ShiSanLan);
		else
			tp.style = static_cast<int>(MahjongGenre::HuStyle::QiXingShiSanLan);
		if (ziNums < 7 && ziNums > 3) {
			// 4、5：13烂差一个字，6：七星13烂差一个字
			for (unsigned int i = 0; i < 7; ++i) {
				if (arrZiNums[i] > 0)
					continue;
				switch (i)
				{
				case 0:
					tp.tile = MahjongTile::Tile(MahjongTile::Pattern::Dong);
					break;
				case 1:
					tp.tile = MahjongTile::Tile(MahjongTile::Pattern::Nan);
					break;
				case 2:
					tp.tile = MahjongTile::Tile(MahjongTile::Pattern::Xi);
					break;
				case 3:
					tp.tile = MahjongTile::Tile(MahjongTile::Pattern::Bei);
					break;
				case 4:
					tp.tile = MahjongTile::Tile(MahjongTile::Pattern::Zhong);
					break;
				case 5:
					tp.tile = MahjongTile::Tile(MahjongTile::Pattern::Fa);
					break;
				case 6:
					tp.tile = MahjongTile::Tile(MahjongTile::Pattern::Bai);
					break;
				default:
					break;
				}
				tps.push_back(tp);
			}
		}
		if (ziNums > 4) {
			if (ziNums < 7)
				tp.style = static_cast<int>(MahjongGenre::HuStyle::ShiSanLan);
			else
				tp.style = static_cast<int>(MahjongGenre::HuStyle::QiXingShiSanLan);
			MahjongTile::TileArray arrAdds;
			FindShiSanLan(MahjongTile::Pattern::Tong, tongNums, arrTongTiles, arrAdds);
			FindShiSanLan(MahjongTile::Pattern::Tiao, tiaoNums, arrTiaoTiles, arrAdds);
			FindShiSanLan(MahjongTile::Pattern::Wan, wanNums, arrWanTiles, arrAdds);
			MahjongTile::TileArray::const_iterator it = arrAdds.begin();
			while (it != arrAdds.end()) {
				tp.tile = *it;
				tps.push_back(tp);
				++it;
			}
		}
		return true;
	}

	bool MahjongRule::checkQiXiaoDui(const MahjongTileArray& tiles, MahjongGenre::TingPaiArray& tps) const {
		if (tiles.size() != 13)
			return false;

		MahjongTile::Tile tile;
		bool bTest = false;
		bool bSame = false;
		unsigned int i = 0;
		while (i < 13) {
			if (i < 12) {
				if (tiles[i].isSame(tiles[i + 1].getTile())) {
					bSame = true;
					i += 2;
				}
			}
			if (bSame)
				bSame = false;
			else {
				if (bTest)
					return false;
				bTest = true;
				tile = MahjongTile::Tile(tiles[i].getPattern(), tiles[i].getNumber());
				i++;
			}
		}
		bool bFound = false;
		MahjongGenre::TingPaiArray::iterator it = tps.begin();
		while (it != tps.end()) {
			if (it->tile == tile) {
				it->style = static_cast<int>(MahjongGenre::HuStyle::QiXiaoDui);
				bFound = true;
				break;
			}
			++it;
		}
		if (!bFound) {
			MahjongGenre::TingPai tp;
			tp.style = static_cast<int>(MahjongGenre::HuStyle::QiXiaoDui);
			tp.tile = tile;
			tps.push_back(tp);
		}
		return true;
	}

	bool MahjongRule::checkPingHu(const MahjongTileArray& tiles, MahjongGenre::TingPaiArray& tps, PingHuStyle* pStyle) const {
		// 基本原则：任何时候手上余牌的张数必定为3n + 1(n = 0, 1, 2, 3, 4)，否则为相公无法胡牌。
		int cnt = static_cast<int>(tiles.size() % 3);
		if (cnt != 1)
			return false;

		MahjongTile::TileArray tmpTiles;
		int arrNums[10] = { 0 };
		int nums1 = 0;
		int nums2 = 0;
		bool bResult = false;
		MahjongTile::Pattern pat = MahjongTile::Pattern::Invalid;
		for (int i = static_cast<int>(MahjongTile::Pattern::Chun); i <= static_cast<int>(MahjongTile::Pattern::Zhu); i++) {
			pat = static_cast<MahjongTile::Pattern>(i);
			if (getPatternNums(tiles, pat) > 0)	// 有花牌就不可能平胡
				return false;
		}
		for (int i = static_cast<int>(MahjongTile::Pattern::Tong); i < static_cast<int>(MahjongTile::Pattern::Chun); i++) {
			pat = static_cast<MahjongTile::Pattern>(i);
			arrNums[i - static_cast<int>(MahjongTile::Pattern::Tong)] = getPatternNums(tiles, pat);
		}
		cnt = 10;
		int tmp = 0;
		for (int i = 0; i < cnt; i++) {
			tmp = arrNums[i];
			tmp %= 3;
			if (tmp == 1)
				nums1++;
			else if (tmp == 2)
				nums2++;
		}
		// 胡牌之后最终的手牌数量必定是3n+2，
		// 任何一种类型的牌在胡牌之后的总数必定是要么为3n(“n阙”)，要么为3n+2(“n阙+一对将”)
		// 且胡牌后总数3n + 2的牌的种类的数量只能有1个
		if (nums1 > 1) {
			// 对于平胡来说，当前手牌中有两种及以上种类数量的牌的数量为3n+1，肯定无法听牌，因为数量为3n+1是没有办法组成“n阙”及“n阙+一对将”的
			return false;
		}
		else if (nums1 == 1) {
			if (nums2 > 0)
				return false;
		}
		else if (nums2 != 2) {
			// 若nums1 == 0则nums2 != 0，否则当前总数不可能是3n + 1
			// 若nums2 == 1，无法听牌
			// 若nums2 > 2，最终必定有超过一对将，无法听牌(若是7小对，前面已经做了处理)
			return false;
		}
		for (int i = static_cast<int>(MahjongTile::Pattern::Tong); i < static_cast<int>(MahjongTile::Pattern::Chun); i++) {
			tmp = arrNums[i - static_cast<int>(MahjongTile::Pattern::Tong)];
			tmp %= 3;
			if (tmp != 0)
				continue;

			// 所有数量为3n的牌都必须组成"n阙"，否则当前无法听牌
			pat = static_cast<MahjongTile::Pattern>(i);
			getTilesByPattern(tiles, pat, tmpTiles);
			if (!checkPingHu3n(tmpTiles))
				return false;
		}
		if (nums1 == 1) {
			for (int i = static_cast<int>(MahjongTile::Pattern::Tong); i < static_cast<int>(MahjongTile::Pattern::Chun); i++) {
				tmp = arrNums[i - static_cast<int>(MahjongTile::Pattern::Tong)];
				tmp %= 3;
				if (tmp != 1)
					continue;

				pat = static_cast<MahjongTile::Pattern>(i);
				getTilesByPattern(tiles, pat, tmpTiles);
				bResult = checkPingHu(tmpTiles, tps, pat, pStyle);
				break;
			}
		} else {
			// 此时nums1 = 0 && nums2 == 2必定成立
			cnt = 0;
			int arrPat[2] = { 0, 0 };
			int arrRet[2] = { 0, 0 };
			for (int i = static_cast<int>(MahjongTile::Pattern::Tong); i < static_cast<int>(MahjongTile::Pattern::Chun); i++) {
				tmp = arrNums[i - static_cast<int>(MahjongTile::Pattern::Tong)];
				tmp %= 3;
				if (tmp != 2)
					continue;

				pat = static_cast<MahjongTile::Pattern>(i);
				getTilesByPattern(tiles, pat, tmpTiles);
				if (checkPingHu3n2(tmpTiles)) {
					arrRet[nums1] = 1;
					cnt++;
				}
				arrPat[nums1++] = i;
				if (nums1 == 2)
					break;
			}
			if (cnt == 0) {
				// 两种种数量为3n + 2的牌都无法组成"n阙+一对将"，无法听牌
				return false;
			} else if (cnt == 1) {
				// 只有一种数量为3n + 2的牌能组成"n阙+一对将"，则只需要判断另一种数量为3n + 2的牌
				if (arrRet[0] == 0)
					pat = static_cast<MahjongTile::Pattern>(arrPat[0]);
				else
					pat = static_cast<MahjongTile::Pattern>(arrPat[1]);
				getTilesByPattern(tiles, pat, tmpTiles);
				bResult = checkPingHu(tmpTiles, tps, pat, pStyle);
			} else {
				// 两种种数量为3n + 2的牌都能组成"n阙+一对将"
				for (unsigned int i = 0; i < 2; i++) {
					pat = static_cast<MahjongTile::Pattern>(arrPat[i]);
					getTilesByPattern(tiles, pat, tmpTiles);
					if (checkPingHu(tmpTiles, tps, pat, pStyle))
						bResult = true;
				}
			}
		}
		return bResult;
	}

	bool MahjongRule::checkPingHu(const MahjongTile::TileArray& tiles, MahjongGenre::TingPaiArray& tps, MahjongTile::Pattern pat, PingHuStyle* pStyle) const {
		PingHuStyle ps;
		bool bResult = false;
		if (pat > MahjongTile::Pattern::Wan) {
			ps.pai = MahjongTile::Tile(pat);
			ps.style = 0;
			if (checkPingHu(tiles, ps.pai, &ps)) {
				bResult = true;
				tps.push_back(MahjongGenre::TingPai(ps.pai, MahjongGenre::HuStyle::PingHu));
				if (pStyle != nullptr)
					*pStyle = ps;
			}
		} else {
			MahjongTile::Number num = MahjongTile::Number::Invalid;
			for (int j = static_cast<int>(MahjongTile::Number::Yi); j <= static_cast<int>(MahjongTile::Number::Jiu); j++) {
				num = static_cast<MahjongTile::Number>(j);
				ps.pai = MahjongTile::Tile(pat, num);
				ps.style = 0;
				if (checkPingHu(tiles, ps.pai, &ps)) {
					bResult = true;
					tps.push_back(MahjongGenre::TingPai(ps.pai, MahjongGenre::HuStyle::PingHu));
					if (pStyle != nullptr)
						*pStyle = ps;
				}
			}
		}
		return bResult;
	}

	bool MahjongRule::checkPingHu(const MahjongTile::TileArray& tiles, MahjongTile::Tile pai, PingHuStyle* pStyle) const {
		MahjongTile::TileArray arrTiles;
		arrTiles.reserve(tiles.size() + 1);
		MahjongTile::TileArray::const_iterator it = tiles.begin();
		bool bTest = false;
		while (it != tiles.end()) {
			if (!bTest) {
				if (*it > pai) {
					arrTiles.push_back(pai);
					bTest = true;
				}
			}
			arrTiles.push_back(*it);
			++it;
		}
		if (!bTest)
			arrTiles.push_back(pai);

		bool bHu = false;
		int cnt = static_cast<int>(arrTiles.size() % 3);
		if (cnt == 0)
			bHu = checkPingHu3n(arrTiles, pStyle);
		else if (cnt == 2)
			bHu = checkPingHu3n2(arrTiles, pStyle);
		return bHu;
	}

	bool MahjongRule::checkPingHu3n2(const MahjongTile::TileArray& tiles, PingHuStyle* pStyle) const {
		unsigned int cnt = static_cast<unsigned int>(tiles.size());
		if ((cnt % 3) != 2)
			return false;
		MahjongTile::Pattern pat = tiles[0].getPattern();
		if (pat > MahjongTile::Pattern::Wan) {
			if (pStyle != nullptr)
				pStyle->style = static_cast<int>(MahjongGenre::HuStyle::DanDiao);
			return true;
		}
		unsigned int tmp = 0;
		MahjongTile::Tile pai;
		for (unsigned int i = 0; i < cnt; i++) {
			if (tiles[i] == pai)
				tmp++;
			else {
				if (tmp > 1) {
					if (checkPingHu3n2(tiles, pai, pStyle))
						return true;
				}
				tmp = 1;
				pai = tiles[i];
			}
		}
		if (tmp > 1) {
			if (checkPingHu3n2(tiles, pai, pStyle))
				return true;
		}
		return false;
	}

	bool MahjongRule::checkPingHu3n2(const MahjongTile::TileArray& tiles, MahjongTile::Tile pai, PingHuStyle* pStyle) const {
		int nums = 0;
		MahjongTile::TileArray arrTiles;
		arrTiles.reserve(tiles.size());
		MahjongTile::TileArray::const_iterator it = tiles.begin();
		while (it != tiles.end()) {
			if ((nums < 2) && (pai == *it)) {
				nums++;
				++it;
				continue;
			}
			arrTiles.push_back(*it);
			++it;
		}
		bool bTest = checkPingHu3n(arrTiles, pStyle);
		if (bTest && (pStyle != nullptr)) {
			if ((pai == pStyle->pai) && (pStyle->style == 0))
				pStyle->style = static_cast<int>(MahjongGenre::HuStyle::DanDiao);
		}
		return bTest;
	}

	bool MahjongRule::checkPingHu3n(const MahjongTile::TileArray& tiles, PingHuStyle* pStyle) const {
		unsigned int cnt = static_cast<unsigned int>(tiles.size());
		if ((cnt % 3) != 0)
			return false;
		if (tiles.empty())
			return true;
		MahjongTile::Pattern pat = tiles[0].getPattern();
		if (pat > MahjongTile::Pattern::Wan) {
			if (pStyle != nullptr)
				pStyle->style = static_cast<int>(MahjongGenre::HuStyle::DanDiao);
			return true;
		}
		int num = 0;
		int arrNums[9] = { 0 };
		for (unsigned int i = 0; i < cnt; i++) {
			num = static_cast<int>(tiles[i].getNumber());
			num--;
			arrNums[num]++;
		}
		if (pStyle != nullptr)
			pStyle->style = 0;
		return checkPingHu3n(arrNums, pStyle);
	}

	void copyNums(int dstNums[9], const int srcNums[9]) {
		for (unsigned int i = 0; i < 9; i++)
			dstNums[i] = srcNums[i];
	}

	bool MahjongRule::checkPingHu3n(const int arrNums[9], PingHuStyle* pStyle) const {
		// 递归三消法，若最终能将牌数清0，则说明是3n，每层递归消除的阙不能有交集
		bool bTest = true;
		for (int i = 0; i < 9; i++) {
			if ((arrNums[i] % 3) != 0) {
				bTest = false;
				break;
			}
		}
		if (bTest)
			return true;
		bool bTest1 = false;
		bool bTest2 = false;
		int arrTemps[9] = { 0 };
		int s = 0;
		int m = 7;
		int a = 0;
		for (; s < m; s++) {
			if ((arrNums[s] == 0) || (arrNums[s + 1] == 0) || (arrNums[s + 2] == 0))
				continue;

			if (pStyle != nullptr) {
				// 3、7才有可能是边张
				if (((s == 0) && (pStyle->pai.getNumber() == MahjongTile::Number::San)) ||
					((s == 6) && (pStyle->pai.getNumber() == MahjongTile::Number::Qi)))
					pStyle->style = static_cast<int>(MahjongGenre::HuStyle::BianZhang);
				else if ((s + 2) == static_cast<int>(pStyle->pai.getNumber()))
					pStyle->style = static_cast<int>(MahjongGenre::HuStyle::KaZhang);
			}
			bTest1 = true;
			for (int i = 0; i < 4; i++) {
				bTest2 = false;
				copyNums(arrTemps, arrNums);
				arrTemps[s]--;
				arrTemps[s + 1]--;
				arrTemps[s + 2]--;
				a = s + 3 + i;
				while (a < 7) {
					if ((arrNums[a] == 0) || (arrNums[a + 1] == 0) || (arrNums[a + 2] == 0)) {
						a++;
						continue;
					}
					if (!bTest) {
						bTest = true;
						m = a;
					}
					if (pStyle != nullptr) {
						// 7才有可能是边张
						if ((a == 6) && (pStyle->pai.getNumber() == MahjongTile::Number::Qi))
							pStyle->style = static_cast<int>(MahjongGenre::HuStyle::BianZhang);
						else if ((a + 2) == static_cast<int>(pStyle->pai.getNumber()))
							pStyle->style = static_cast<int>(MahjongGenre::HuStyle::KaZhang);
					}
					bTest2 = true;
					arrTemps[a]--;
					arrTemps[a + 1]--;
					arrTemps[a + 2]--;
					a += 3;
				}
				if (bTest2) {
					// 测试单层递归消除所有没有交集的阙(可以检测绝大部分可能性)
					if (checkPingHu3n(arrTemps, pStyle))
						return true;
					bTest1 = false;
				}
			}
			// 测试单层递归只消1阙
			if (bTest1) {
				if (checkPingHu3n(arrTemps, pStyle))
					return true;
			} else {
				// 可以检测特殊情况，例如：333456678999，只能单层递归消除456，若此层中再消除789，则无法得出正确判定
				copyNums(arrTemps, arrNums);
				arrTemps[s]--;
				arrTemps[s + 1]--;
				arrTemps[s + 2]--;
				if (checkPingHu3n(arrTemps, pStyle))
					return true;
			}
		}
		return false;
	}

	int MahjongRule::getTileNums(const MahjongTile::TileArray& tiles, MahjongTile::Tile pai) {
		bool bFound = false;
		unsigned int nums = 0;
		MahjongTile::TileArray::const_iterator it = tiles.begin();
		while (it != tiles.end()) {
			if (*it == pai) {
				bFound = true;
				nums++;
			}
			else if (bFound)
				break;

			++it;
		}
		return nums;
	}

	int MahjongRule::getPatternNums(const MahjongTileArray& tiles, MahjongTile::Pattern pat) {
		bool bFound = false;
		unsigned int nums = 0;
		MahjongTileArray::const_iterator it = tiles.begin();
		while (it != tiles.end()) {
			if (it->getPattern() == pat) {
				nums++;
				bFound = true;
			}
			else if (bFound)
				break;
			++it;
		}
		return nums;
	}

	void MahjongRule::getTilesByPattern(const MahjongTileArray& tiles, MahjongTile::Pattern pat, MahjongTile::TileArray& tmpTiles) {
		tmpTiles.clear();

		MahjongTileArray::const_iterator it = tiles.begin();
		while (it != tiles.end()) {
			if (it->getPattern() == pat)
				tmpTiles.push_back(it->getTile());

			++it;
		}
	}

	void MahjongRule::getTileArrayString(const MahjongTileArray& tiles, std::string& text) {
		text.clear();
		std::string strTmp;
		MahjongTileArray::const_iterator it = tiles.begin();
		while (it != tiles.end()) {
			it->getTile().toString(strTmp);
			text += strTmp;
			++it;
		}
	}

	bool MahjongRule::checkQingYiSe(const MahjongTileArray& tiles) const {
		if (tiles.empty())
			return false;

		bool bFirst = true;
		MahjongTile::Pattern pat = MahjongTile::Pattern::Invalid;
		MahjongTileArray::const_iterator it = tiles.begin();
		while (it != tiles.end()) {
			if (bFirst) {
				bFirst = false;
				pat = it->getPattern();
			}
			else if (pat != it->getPattern())
				return false;
			++it;
		}
		return true;
	}

	bool MahjongRule::checkZiYiSe(const MahjongTileArray& tiles) const {
		MahjongTile::Pattern pat = MahjongTile::Pattern::Invalid;
		MahjongTileArray::const_iterator it = tiles.begin();
		while (it != tiles.end()) {
			pat = it->getPattern();
			if ((pat < MahjongTile::Pattern::Dong) || (pat > MahjongTile::Pattern::Bai))
				return false;
			++it;
		}
		return true;
	}

	bool MahjongRule::checkPengPengHu(const MahjongTileArray& tiles) const {
		bool bTest2 = false;
		int nums = 0;
		MahjongTile::Tile t;
		MahjongTileArray::const_iterator it = tiles.begin();
		while (it != tiles.end()) {
			if (t == it->getTile())
				nums++;
			else {
				if (nums == 1)
					return false;		// 有单个的牌肯定不是碰碰胡
				else if (nums == 2) {
					if (bTest2)
						return false;	// 含有两对及两对以上的牌肯定不是碰碰胡
					bTest2 = true;
				}
				nums = 1;
				t = it->getTile();
			}
			++it;
		}
		return true;
	}

	bool MahjongRule::checkQiXiaoDui(const MahjongTileArray& tiles) const {
		if (tiles.size() != 14)
			return 0;
		for (unsigned int i = 0; i < 14; i += 2) {
			if (tiles[i].getTile() != tiles[i + 1].getTile())
				return false;
		}
		return true;
	}

	int MahjongRule::checkHaoHuaQiXiaoDui(const MahjongTileArray& tiles) const {
		if (tiles.size() != 14)
			return 0;

		int nums = 0;
		int ret = 0;
		MahjongTile::Tile tile;
		MahjongTileArray::const_iterator it = tiles.begin();
		while (it != tiles.end()) {
			if (tile == it->getTile())
				nums++;
			else {
				if (nums == 4)
					ret++;
				else if (nums == 3 || nums == 1)
					return ret;
				nums = 1;
				tile = it->getTile();
			}
			++it;
		}
		if (nums == 4)
			ret++;
		return ret;
	}

	bool MahjongRule::checkHuStyleEx(const MahjongTileArray& tiles, MahjongGenre::TingPaiArray& tps, MahjongAvatar* pAvatar) const {
		return false;
	}
}