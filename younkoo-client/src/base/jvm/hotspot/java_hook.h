﻿#pragma once
#include "include_header.h"
#include "utility/jvm_internal.h"
#include "break/jvm_break_points.h"
#include "utility/vm_helper.h"
#include "classes/c_method.h"
#include "classes/java_thread.h"
#include "classes/instance_klass.h"
#include "classes/const_pool.h"


namespace JavaHook {
	namespace JVM {
		bool Init(JNIEnv* env);
		bool clean();
	}

}
