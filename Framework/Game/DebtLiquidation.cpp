// DebtLiquidation.cpp

#include "Base/Log.h"
#include "Base/BaseUtils.h"
#include "DebtLiquidation.h"

#include <sstream>
#include <list>

namespace NiuMa
{
	DebtNode::DebtNode(int id, double capital)
		: _id(id)
		, _capital(capital)
	{}

	DebtNode::~DebtNode()
	{}

	int DebtNode::getId() const {
		return _id;
	}

	double DebtNode::getCapital() const {
		return _capital;
	}

	void DebtNode::setCapital(double cap) {
		_capital = cap;
	}

	void DebtNode::addCapital(double cap) {
		_capital += cap;
	}

	void DebtNode::tally(DebtNode* node, double amount) {
		if (node == NULL)
			return;

		std::unordered_map<DebtNode*, double>::iterator it = _debtSheet.find(node);
		if (it != _debtSheet.end())
			(it->second) += amount;
		else
			_debtSheet.insert(std::make_pair(node, amount));
	}

	bool DebtNode::receipt(DebtNode* node) {
		if (node == NULL)
			return false;

		// 只有债务人才有资格还款
		try {
			std::unordered_map<DebtNode*, double>::iterator it = _debtSheet.find(node);
			if (it == _debtSheet.end())
				throw std::runtime_error("Failed to receipt, can't find the specified node.");
			else if ((it->second) > 0.0)
				throw std::runtime_error("Failed to receipt, the specified node is not debtor.");
			else {
				_capital -= (it->second);
				_debtSheet.erase(it);
			}
		}
		catch (std::exception& ex) {
			LOG_ERROR(ex.what());
			return false;
		}
		return true;
	}

	void DebtNode::repayAll() {
		std::unordered_map<DebtNode*, double>::iterator it = _debtSheet.begin();
		while (it != _debtSheet.end()) {
			if (((it->second) > 0.0) && (it->first)->receipt(this)) {
				_capital -= (it->second);
				it = _debtSheet.erase(it);
			}
			else
				++it;
		}
	}

	bool DebtNode::repay(DebtNode* node) {
		if (node == NULL)
			return false;

		// 只向债权人还款
		try {
			std::unordered_map<DebtNode*, double>::iterator it = _debtSheet.find(node);
			if (it == _debtSheet.end())
				throw std::runtime_error("Failed to repay, can't find the specified node.");
			else if ((it->second) < 0.0)
				throw std::runtime_error("Failed to repay, the specified node is not creditor.");
			else if ((it->first)->receipt(this)) {
				_capital -= (it->second);
				_debtSheet.erase(it);
			}
		}
		catch (std::exception& ex) {
			LOG_ERROR(ex.what());
			return false;
		}
		return true;
	}

	void DebtNode::erase(DebtNode* node) {
		std::unordered_map<DebtNode*, double>::iterator it = _debtSheet.find(node);
		if (it != _debtSheet.end())
			_debtSheet.erase(it);
	}

	void DebtNode::erase(const std::unordered_map<int, DebtNode*>& debtNet) {
		DebtNode* node = NULL;
		std::unordered_map<DebtNode*, double>::iterator it1 = _debtSheet.begin();
		std::unordered_map<int, DebtNode*>::const_iterator it2;
		while (it1 != _debtSheet.end()) {
			node = it1->first;
			it2 = debtNet.find(node->getId());
			if (it2 != debtNet.end()) {
				it1 = _debtSheet.erase(it1);
				node->erase(this);
			}
			else
				it1++;
		}
	}

	std::unordered_map<DebtNode*, double>& DebtNode::getDebtSheet() {
		return _debtSheet;
	}

	const std::unordered_map<DebtNode*, double>& DebtNode::getDebtSheet() const {
		return _debtSheet;
	}

	double DebtNode::getDebt() const {
		double profit = 0.0;
		std::unordered_map<DebtNode*, double>::const_iterator it = _debtSheet.begin();
		while (it != _debtSheet.end()) {
			profit += (it->second);
			++it;
		}
		return profit;
	}

	const double DebtLiquidation::TOLERANCE = 1.0E-6;

	DebtLiquidation::DebtLiquidation()
	{}

	DebtLiquidation::~DebtLiquidation()
	{}

	bool DebtLiquidation::operator()(std::unordered_map<int, DebtNode*>& debtNet) {
		if (debtNet.empty())
			return true;

		DebtNode* node = NULL;
		std::unordered_map<int, DebtNode*> positiveNodes;	// 正收益的全部节点
		std::unordered_map<int, DebtNode*> negativeNodes;	// 负收益的全部节点
		std::unordered_map<int, DebtNode*>::const_iterator it = debtNet.begin();
		while ( it != debtNet.end()) {
			node = (it->second);
			// 净负债为负，即获得正收益
			if (node->getDebt() < 0.0)
				positiveNodes.insert(std::make_pair(node->getId(), node));
			else
				negativeNodes.insert(std::make_pair(node->getId(), node));
			++it;
		}
		if (positiveNodes.empty()) {
			// 债务网中没有任何节点获得正收益，只有两种可能：
			// 1、债务网内所有节点都既无欠款也无应收账款，即债务网内所有节点间都无债务关系
			// 2、债务网内所有节点的欠款与应收账款之和为0，等价于债务网内所有节点间都无债务关系
			// 注：此时的债务网(本函数的传进来的参数debtNet)与外部之间的债务都已经剥离，否则无法得出
			// 上述两个结论!!
			return true;
		}
		// 获得正收益的节点先尝还全部债务，偿还完后正收益的节点只有应收账款无负债
		it = positiveNodes.begin();
		while (it != positiveNodes.end()) {
			node = (it->second);
			node->repayAll();
			++it;
		}
		// 剥离全部负收益节点与全部正收益节点之间的债务，负收益节点之间的子债务网做内部清算
		DebtNode* tmpNode = NULL;
		std::unordered_map<int, DebtNode*> subDebtNet;
		std::unordered_map<DebtNode*, double>::const_iterator it2;
		std::unordered_map<int, DebtNode*>::const_iterator it1 = negativeNodes.begin();
		while (it1 != negativeNodes.end()) {
			node = (it1->second);
			tmpNode = new DebtNode(node->getId(), node->getCapital());
			subDebtNet.insert(std::make_pair(node->getId(), tmpNode));
			++it1;
		}
		// test为true则子网内有债务关系
		bool test = false;
		it1 = negativeNodes.begin();
		while (it1 != negativeNodes.end()) {
			node = (it1->second);
			it = subDebtNet.find(node->getId());
			if (it != subDebtNet.end())
				tmpNode = it->second;
			else
				return false;
			const std::unordered_map<DebtNode*, double>& debtSheet = node->getDebtSheet();
			it2 = debtSheet.begin();
			while (it2 != debtSheet.end()) {
				it = subDebtNet.find((it2->first)->getId());
				if (it != subDebtNet.end()) {
					test = true;
					tmpNode->tally(it->second, it2->second);
				}
				++it2;
			}
			++it1;
		}
		if (test) {
			if (!operator()(subDebtNet)) {
				releaseDebtNet(subDebtNet);
				return false;
			}
			// 消除子债务网内全部节点间的债务关系
			it1 = negativeNodes.begin();
			while (it1 != negativeNodes.end()) {
				node = (it1->second);
				it = subDebtNet.find(node->getId());
				if (it != subDebtNet.end())
					tmpNode = it->second;
				else
					return false;
				node->setCapital(tmpNode->getCapital());
				node->erase(subDebtNet);
				++it1;
			}
		}
		releaseDebtNet(subDebtNet);
		// 执行到这里，负收益节点之间清算完毕，已经没有债务关系，全部负收益节点无任何应收账款且仅对
		// 正收益节点有负债，全部正收益节点无任何负债且仅对负收益节点有应收账款。且债务网内已经无三
		// 角债，只有简单的单向债务关系。此时再对全部节点进行清算，若资不抵债则按负债比例偿付。
		double debt = 0.0;
		double cap = 0.0;
		double tmp = 0.0;
		it1 = negativeNodes.begin();
		while (it1 != negativeNodes.end()) {
			node = (it1->second);
			debt = node->getDebt();
			cap = node->getCapital();
			if (debt > cap) {
				// 债务大于现金，不足以偿债只能按比例偿付
				std::unordered_map<DebtNode*, double>& debtSheet = node->getDebtSheet();
				it2 = debtSheet.begin();
				while (it2 != debtSheet.end()) {
					if (debt > 0.0) {
						tmp = cap * (it2->second) / debt;
						(it2->first)->addCapital(tmp);
					}
					(it2->first)->erase(node);
					++it2;
				}
				node->setCapital(0.0);
				debtSheet.clear();
			}
			else {
				// 现金足以偿债
				node->repayAll();
			}
			++it1;
		}
		return true;
	}

	void DebtLiquidation::releaseDebtNet(std::unordered_map<int, DebtNode*>& debtNet) {
		std::unordered_map<int, DebtNode*>::iterator it = debtNet.begin();
		while (it != debtNet.end()) {
			if ((it->second) != NULL)
				delete (it->second);
			++it;
		}
		debtNet.clear();
	}

	void DebtLiquidation::printDebtNet(const std::unordered_map<int, DebtNode*>& debtNet, std::string& text) {
		std::ostringstream os;
		std::unordered_map<int, DebtNode*>::const_iterator it = debtNet.begin();
		std::unordered_map<DebtNode*, double>::const_iterator it1;
		while (it != debtNet.end()) {
			os << "{id:" << it->first << ", debt sheet:{";
			const std::unordered_map<DebtNode*, double>& debtSheet = (it->second)->getDebtSheet();
			it1 = debtSheet.begin();
			while (it1 != debtSheet.end()) {
				os << "{id:" << (it1->first)->getId() << ", amount:" << (it1->second) << "},";
				++it1;
			}
			os << "}},";
			++it;
		}
		text = os.str();
	}
}