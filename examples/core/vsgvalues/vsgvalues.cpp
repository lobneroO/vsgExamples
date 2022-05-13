#include <vsg/core/Auxiliary.h>
#include <vsg/core/Object.h>
#include <vsg/core/Value.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ConstVisitor.h>
#include <vsg/io/stream.h>

#include <algorithm>
#include <iostream>
#include <typeinfo>

// helper function to simplify iteration through any user objects/values assigned to an object.
template<typename P, typename F>
void for_each_user_object(P object, F functor)
{
    if (object->getAuxiliary())
    {
        using ObjectMap = vsg::Auxiliary::ObjectMap;
        ObjectMap& objectMap = object->getAuxiliary()->getObjectMap();
        for (ObjectMap::iterator itr = objectMap.begin();
             itr != objectMap.end();
             ++itr)
        {
            functor(*itr);
        }
    }
}

namespace engine
{
    // provide a custom struct for storing data, such as for passing to the GPU as a uniform value
    struct property
    {
        float speed = 0.0f;
    };

    // declare a Value object for the property struct
    using propertyValue = vsg::Value<property>;
}
// provide a vsg::type_name<> for our custom propertyValue class using EVSG_type_name() macro, note must be done in global scope.
EVSG_type_name(engine::propertyValue)

int main(int /*argc*/, char** /*argv*/)
{
    vsg::ref_ptr<vsg::Object> object(new vsg::Object());
    object->setValue("name", "Name field contents");
    object->setValue("time", 10.0);
    object->setValue("size", 3.1f);
    object->setValue("count", 5);
    object->setValue("pos", 4u);

    if (object->getAuxiliary())
    {
        struct VisitValues : public vsg::Visitor
        {
            using Visitor::apply;

            void apply(vsg::Object& object) override
            {
                std::cout << "Object, " << typeid(object).name() << std::endl;
            }

            void apply(vsg::intValue& value) override
            {
                std::cout << "intValue,  value = " << value << std::endl;
            }

            void apply(vsg::uintValue& value) override
            {
                std::cout << "uintValue,  value = " << value << std::endl;
            }

            void apply(vsg::floatValue& value) override
            {
                std::cout << "floatValue, value  = " << value << std::endl;
            }

            void apply(vsg::doubleValue& value) override
            {
                std::cout << "doubleValue, value  = " << value << std::endl;
            }

            void apply(vsg::stringValue& value) override
            {
                std::cout << "stringValue, value  = " << value.value() << std::endl;
            }
        };

        VisitValues visitValues;

        std::cout << "Object has Auxiliary so check it's ObjectMap for our values. " << object->getAuxiliary() << std::endl;
        for (vsg::Auxiliary::ObjectMap::iterator itr = object->getAuxiliary()->getObjectMap().begin();
             itr != object->getAuxiliary()->getObjectMap().end();
             ++itr)
        {
            std::cout << "   key[" << itr->first << "] ";
            itr->second->accept(visitValues);
        }

        std::cout << "Use for_each " << std::endl;
        std::for_each(object->getAuxiliary()->getObjectMap().begin(), object->getAuxiliary()->getObjectMap().end(), [&visitValues](vsg::Auxiliary::ObjectMap::value_type& key_value) {
            std::cout << "   key[" << key_value.first << "] ";
            key_value.second->accept(visitValues);
        });

        std::cout << "for_each_user_object " << std::endl;
        for_each_user_object(object, [&visitValues](vsg::Auxiliary::ObjectMap::value_type& key_value) {
            std::cout << "   key[" << key_value.first << "] ";
            key_value.second->accept(visitValues);
        });
    }

    // create a propertyValue object wrapper around a engine::property struct.
    auto my_property = engine::propertyValue::create(engine::property{10.0});
    std::cout<<"\nmy_property = "<<my_property<<", className = "<<my_property->className()<<std::endl;
    std::cout<<"    after constructor my_property->value->speed = "<<my_property->value().speed<<std::endl;

    // we can modify the value struct by getting the value from the value obejct and mofifying it:
    my_property->value().speed += 2.0f;
    std::cout<<"    after increment my_property->value->speed = "<<my_property->value().speed<<std::endl;

    // or get a referenece to underlying engine::property value held by the my_property object
    auto& prop = my_property->value();
    prop.speed *= 2.0f; // modifying a member of the struct
    std::cout<<"    after multiplication my_property->value->speed = "<<prop.speed<<std::endl;

    return 0;
}
