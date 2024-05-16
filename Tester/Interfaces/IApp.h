#pragma once

#include <memory>
#include <type_traits>
#include <vector>

template <class Derived> class IApp
{
public:
	IApp() = default;

	static std::shared_ptr<Derived> Get();

	virtual void Init(const std::vector<std::string> &args) = 0;

	virtual void Run() = 0;

	virtual void Destroy() = 0;

private:
	static std::shared_ptr<Derived> m_this;
};

template <class Derived> std::shared_ptr<Derived> IApp<Derived>::m_this = nullptr;

template <class Derived> std::shared_ptr<Derived> IApp<Derived>::Get()
{
	if (!m_this)
		m_this = std::make_shared<Derived>();
	return m_this;
}
