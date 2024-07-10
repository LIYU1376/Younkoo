﻿#include "Object.h"


using namespace Wrapper;

Object::Object(const Object& other_Object) {
	this->instance = std::make_shared<JNI::EmptyMembers>(*other_Object.instance.get());
}

Object::Object(const JNI::EmptyMembers& obj_wrap)
{
	this->instance = std::make_shared<JNI::EmptyMembers>(obj_wrap);
}


bool Object::operator ==(const Object& other_Object)
{
	return this->getObject() == other_Object.getObject() || JNI::get_env()->IsSameObject(this->getObject(), other_Object.getObject());
}

Object Object::operator=(const Object& other_Object)
{
	this->instance = std::make_shared<JNI::EmptyMembers>(*other_Object.instance.get());
	return *this;
}

bool Object::isEqualTo(const Object& other_Object)
{
	return JNI::get_env()->IsSameObject(this->getObject(), other_Object.getObject());
}


bool Object::isNULL()
{
	return JNI::get_env()->IsSameObject(this->getObject(), NULL);
}

jclass Object::getClass()
{
	if (!this->instance) return nullptr;

	return this->instance->owner_klass;
}

jobject Object::getObject()
{
	if (!this->instance) return nullptr;

	return this->instance->object_instance;
}

jclass Object::getClass() const
{
	if (!this->instance) return nullptr;

	return this->instance->owner_klass;
}

jobject Object::getObject() const
{
	if (!this->instance) return nullptr;

	return this->instance->object_instance;
}


Object::~Object()
{

}