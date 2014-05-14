#pragma once

#include "Common.h"

namespace Library
{
	template <class T>
	class Factory
	{
	public:		
		virtual ~Factory();

		virtual const std::string ClassName() const = 0;
		virtual T* Create() const = 0;

		static Factory<T>* Find(const std::string& className);
		static T* Create(const std::string& className);		

		static typename std::map<std::string, Factory<T>*>::iterator Begin();
		static typename std::map<std::string, Factory<T>*>::iterator End();

	protected:		
		static void Add(Factory<T>* const factory);
		static void Remove(Factory<T>* const factory);

	private:
		static std::map<std::string, Factory<T>*> sFactories;
	};

	template <class T>
	std::map<std::string, Factory<T>*> Factory<T>::sFactories;

	template <class T>
	Factory<T>::~Factory()
	{
	}

	template <class T>
	typename std::map<std::string, Factory<T>*>::iterator Factory<T>::Begin()
	{
		return sFactories.begin();
	}

	template <class T>
	typename std::map<std::string, Factory<T>*>::iterator Factory<T>::End()
	{
		return sFactories.end();
	}

	template <class T>
	Factory<T>* Factory<T>::Find(const std::string& className)
	{
		Factory<T>* foundFactory = NULL;

		std::map<std::string, Factory<T>*>::iterator it = sFactories.find(className);
		if (it != sFactories.end())
		{
			foundFactory = it->second;
		}

		return foundFactory;
	}

	template <class T>
	T* Factory<T>::Create(const std::string& className)
	{
		T* product = NULL;

		Factory<T>* factory = Find(className);
		if (factory != NULL)
		{
			product = factory->Create();
		}

		return product;
	}

	template <class T>
	void Factory<T>::Add(Factory<T>* const factory)
	{
		if (Find(factory->ClassName()) == NULL)
		{
			sFactories.insert(std::pair<std::string, Factory<T>*>(factory->ClassName(), factory));
		}
	}

	template <class T>
	void Factory<T>::Remove(Factory<T>* const factory)
	{
		sFactories.erase(factory->ClassName());
	}

	#define ConcreteFactory(ConcreteProductT, AbstractProductT)			\
	class ConcreteProductT ## Factory : public Factory<AbstractProductT>\
	{																	\
	public:																\
		ConcreteProductT ## Factory()									\
		{																\
			Add(this);													\
		}																\
																		\
		~ConcreteProductT ## Factory()									\
		{																\
			Remove(this);												\
		}																\
																		\
		virtual const std::string ClassName() const						\
		{																\
			return std::string( #ConcreteProductT );					\
		}																\
																		\
		virtual AbstractProductT* Create() const						\
		{																\
			AbstractProductT* product = new ConcreteProductT();			\
			return product;												\
		}																\
	};
}
