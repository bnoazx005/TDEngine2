#include <TDEngine2.h>



namespace Test
{
	auto typeHash = TDE2_TYPE_ID(Foo); ///< TDE2_TYPE_ID should be allowed from any namespace
	TDE2_REGISTER_TYPE(Foo) ///< TDE2_REGISTER_TYPE is usually used within struct/class scopes, but it should fit well in a static function manner

	
	struct IFoo
	{
		virtual TDEngine2::TypeId GetComponentTypeId() = 0;
	};

	struct CConcreteFoo : IFoo
	{
		TDE2_REGISTER_COMPONENT_TYPE(CConcreteFoo);
	};
	

	struct IFooSystem
	{
		virtual TDEngine2::TypeId GetSystemType() const = 0;
		virtual const std::string& GetName() const = 0;
	};


	struct CConcreteSystem: IFooSystem
	{
		TDE2_SYSTEM(CConcreteSystem)
	};


	/*!
		\brief The code below tests macros for declaring tag-components within an independent namespace
	*/

	TDE2_DECLARE_FLAG_COMPONENT(Tag)
	TDE2_DEFINE_FLAG_COMPONENT(Tag)
}