/*
 * NodeIterator.h
 *
 *  Created on: Feb 29, 2016
 *      Author: max
 */

#ifndef NODEITERATOR_H_
#define NODEITERATOR_H_

#include <iterator>
#include "nodes/NodeAddressContent.h"

template <unsigned int DIM>
class NodeIterator : public std::iterator<std::input_iterator_tag, NodeAddressContent<DIM>> {
public:
	NodeIterator();
	NodeIterator(unsigned long address);
	virtual ~NodeIterator();
	bool operator==(const NodeIterator& rhs);
	bool operator!=(const NodeIterator& rhs);

	virtual void setAddress(size_t address);
	virtual NodeIterator<DIM>& operator++();
	virtual NodeIterator<DIM> operator++(int);
	virtual NodeAddressContent<DIM> operator*();

protected:
	unsigned long address_;
	bool reachedEnd_;
};

#include <stdexcept>

template <unsigned int DIM>
NodeIterator<DIM>::NodeIterator() : address_(0), reachedEnd_(false) {
}

template <unsigned int DIM>
NodeIterator<DIM>::NodeIterator(unsigned long address) : address_(address), reachedEnd_(false) {
}

template <unsigned int DIM>
NodeIterator<DIM>::~NodeIterator() { }

template <unsigned int DIM>
bool NodeIterator<DIM>::operator==(const NodeIterator<DIM>& rhs) {
	 return address_ == rhs.address_;
}

template <unsigned int DIM>
bool NodeIterator<DIM>::operator!=(const NodeIterator& rhs) {
	 return address_ != rhs.address_;
}

template <unsigned int DIM>
NodeIterator<DIM>& NodeIterator<DIM>::operator++() {
	throw std::runtime_error("subclass should implement this");
}

template <unsigned int DIM>
NodeIterator<DIM> NodeIterator<DIM>::operator++(int) {
	throw std::runtime_error("subclass should implement this");
}

template <unsigned int DIM>
NodeAddressContent<DIM> NodeIterator<DIM>::operator*() {
	throw std::runtime_error("subclass should implement this");
}

template <unsigned int DIM>
void NodeIterator<DIM>::setAddress(size_t address) {
	throw std::runtime_error("subclass should implement this");
}


#endif /* NODEITERATOR_H_ */
