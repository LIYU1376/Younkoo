﻿#pragma once
#include <Windows.h>
#include <SDK.hpp>
#include <wrapper/versions/1_18_1/net/minecraft/client/renderer/ClientRenderer.h>
#include <JVM.hpp>
#include  <hotspot/java_hook.h>
#include <hotspot/break/byte_code_info.h>
#include <hotspot/classes/compile_task.h>
#include <utils/Pattern.h>

#include <format>

namespace RenderSystemHook {
	inline std::vector<java_hotspot::method*> methods_being_hooked;
	inline void detour_invoke_compiler_on_method(java_hotspot::compile_task* task);
	inline TitanHook<decltype(&detour_invoke_compiler_on_method)> _invoke_compiler_on_method_hook;
	inline void cleanHook() {
		for (auto ptr : methods_being_hooked)
		{
			auto method = reinterpret_cast<java_hotspot::method*>(ptr);
			method->remove_all_break_points();
		}
		JavaHook::JVM::clean();
		_invoke_compiler_on_method_hook.RemoveHook();
	}
	struct BytecodeInfo
	{
		BytecodeInfo() {  };
		java_runtime::bytecodes opcode;
		std::vector<int> operands;
		int index = 0;
	};
	inline void detour_invoke_compiler_on_method(java_hotspot::compile_task* task) {
		//TODO : hook ok_to_inline to improve performance
		auto method = task->get_method();
		auto current_pool_holder = method->get_const_method()->get_constants()->get_pool_holder();
		//std::cout << "Compiling method :" << method->get_name() << std::endl;
		for (auto pMethod : methods_being_hooked)
		{


			if (current_pool_holder == pMethod->get_const_method()->get_constants()->get_pool_holder())
			{
				std::cout << "Force break compile of " << static_cast<java_hotspot::instance_klass*>(current_pool_holder)->get_name()->to_string() << std::endl;
				return;
			}
		}

		_invoke_compiler_on_method_hook.GetOrignalFunc()(task);
	}
	inline uintptr_t plocalvariable_table_length_addr = 0;
	inline unsigned short* localvariable_table_length_addr(java_hotspot::const_method* cm) {
		auto func = (decltype(&localvariable_table_length_addr))plocalvariable_table_length_addr;
		return func(cm);
	}
	inline void hook_invoke_compiler_on_method() {
		auto jvm = (uintptr_t)GetModuleHandleA("jvm.dll");

		//CompileBroker::invoke_compiler_on_method(CompileTask *task)
		auto pinvoke_compiler_on_method = (void*)CUtil_Pattern::Find(jvm, "48 89 5C 24 10 48 89 74 24 18 48 89 7C 24 20 55 41 54 41 55 41 56 41 57 48 8D AC 24 80 FA FF FF 48 81 EC 80 06 00 00 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 70 05 00 00 33 D2");// this function will call    __dyn_tls_on_demand_init and doing something like NtCurrentTeb()->ThreadLocalStoragePointer + (unsigned int)TlsIndex);
		if (!pinvoke_compiler_on_method)
		{
			MessageBox(0, L"Failed to get the address of invoke_compiler_on_method", 0, 0);
		}
		_invoke_compiler_on_method_hook.InitHook(pinvoke_compiler_on_method, detour_invoke_compiler_on_method);
		_invoke_compiler_on_method_hook.SetHook();


		//plocalvariable_table_length_addr = CUtil_Pattern::Find(jvm, "48 83 EC ? 0F B7 51 ? 4C 8B C1");

	}
	inline void applyHook() {
		if (SRGParser::get().GetVersion() != Versions::FORGE_1_18_1)
			return;

		jclass klass = JNI::find_class(V1_18_1::GameRenderer::get_name());

		JavaHook::JVM::Init(JNI::get_env());

		//if we dont hook it, when method is going to be inlined,it will crahes with "error: unhandled bytecode".
		hook_invoke_compiler_on_method();

		JVM::get().jvmti->RetransformClasses(1, &klass);
		auto mid = (jmethodID)V1_18_1::GameRenderer::static_obj().renderLevel;
		const auto method = *reinterpret_cast<java_hotspot::method**>(mid);
		methods_being_hooked.emplace_back(method);
		method->set_dont_inline(true);
		const auto access_flags = method->get_access_flags();
		access_flags->set_not_c1_compilable();
		access_flags->set_not_c2_compilable();
		access_flags->set_not_c2_osr_compilable();
		access_flags->set_queued_for_compilation();

		const auto constants_pool = method->get_const_method()->get_constants();

		auto const_method = method->get_const_method();

		auto bytecodes = const_method->get_bytecode();
		const size_t bytecodes_length = bytecodes.size();
		auto* holder_klass = static_cast<java_hotspot::instance_klass*>(constants_pool->get_pool_holder());
		int bytecodes_index = 0;
		std::cout << "length :" << bytecodes_length << std::endl;
		std::cout << "bytecode addr " << (void*)const_method->get_bytecode_start() << std::endl;
		std::list<BytecodeInfo> method_block;
		while (bytecodes_index < bytecodes_length) {
			const auto bytecode = static_cast<java_runtime::bytecodes>(bytecodes[bytecodes_index]);
			auto opcodes = java_runtime::bytecode::bytecode(bytecodes.data() + bytecodes_index);
			BytecodeInfo current_bytecode;
			current_bytecode.opcode = bytecode;
			current_bytecode.index = bytecodes_index;
			{
				auto name = java_runtime::bytecode_names[bytecode];
				std::cout << "(" << bytecodes_index << ")  " << name << " {";
				auto length = opcodes.get_length();
				for (int i = 0; i < length - 1; i++)
				{
					if (i == 0)
					{
						int opreand = (int)bytecodes[bytecodes_index + i];
						current_bytecode.operands.push_back(opreand);
						std::cout << opreand;
					}
					else {

						int opreand = (int)bytecodes[bytecodes_index + i];
						current_bytecode.operands.push_back(opreand);
						std::cout << " , " << (int)bytecodes[bytecodes_index + i];
					}
				}
				method_block.push_back(current_bytecode);
				std::cout << "}" << std::endl;
				bytecodes_index += length;

			}
		}

		for (auto& bytecode_info : method_block)
		{
			if (bytecode_info.opcode == java_runtime::bytecodes::_return_register_finalizer && bytecode_info.operands[0] == 231 && bytecode_info.operands[1] == 158) // ldc "hand"
			{
				std::cout << "setting bytecode on index " << bytecode_info.index << " whose original byte is " << (int)bytecode_info.opcode << " method : " << method << std::endl;
				auto* info = jvm_internal::breakpoint_info::create(method, bytecode_info.index);
				info->set_orig_bytecode(bytecode_info.opcode);
				info->set_next(holder_klass->get_breakpoints());
				holder_klass->set_breakpoints(info);
				jvm_break_points::set_breakpoint_with_original_code(method, bytecode_info.index, static_cast<std::uint8_t>(bytecode_info.opcode), [mid](break_point_info* bp) -> void
					{
						//auto stack = bp->java_thread->get_operand_stack();
						const auto orginal_state = bp->java_thread->get_thread_state();
						bp->java_thread->set_thread_state(JavaThreadState::_thread_in_native);
						JNI::set_thread_env(bp->java_thread->get_jni_envoriment());

						auto parma = (uintptr_t*)bp->get_parameters();
						auto gameRender = (jobject)bp->get_parameter(0);
						auto p_109090_ = *(float*)bp->get_parameter(1);
						long p_109091_ = *bp->lload(2);
						auto p_109092_ = (jobject)bp->get_parameter(3);

						auto matrix4f = (jobject)bp->get_parameter(13);

#ifdef DEBUG


						{
							jthread thread;
							JVM::get().jvmti->GetCurrentThread(&thread);
							char out[1024];
							jint param_size = 0;

							JVM::get().jvmti->GetArgumentsSize(mid, &param_size);
							printf("method_entry: %s%s%, param_size:%d\n", bp->method->get_signature().c_str(), bp->method->get_name().c_str(), param_size);

							jint entry_count = 0;
							jvmtiLocalVariableEntry* table_ptr = NULL;
							jlocation cur_loc{};
							jmethodID methoid;
							auto err = JVM::get().jvmti->GetLocalVariableTable(mid, &entry_count, &table_ptr);
							err = JVM::get().jvmti->GetFrameLocation(thread, 0, &methoid, &cur_loc);
							for (int j = 0; j < entry_count; j++) {
								if (table_ptr[j].start_location > cur_loc) break;


								if (table_ptr[j].signature[0] == 'L') { //   fully-qualified-class
									jobject param_obj{};
									jlong param_obj_tag = 0;

									JVM::get().jvmti->GetLocalObject(thread, 0, table_ptr[j].slot, &param_obj); // frame at depth zero is the current frame                  
									if (param_obj) JVM::get().jvmti->GetTag(param_obj, &param_obj_tag);
									if (param_obj_tag == 0) {

									}
									printf(", param_obj_tag: %ld", param_obj_tag);
									printf(", slot:%d, start:%ld, cur:%ld, param:%s%s\n", table_ptr[j].slot, table_ptr[j].start_location, cur_loc, table_ptr[j].signature, table_ptr[j].name);
									if (param_obj)
									{
										JNI::get_env()->DeleteLocalRef(param_obj);
									}
									JVM::get().jvmti->Deallocate(reinterpret_cast<unsigned char*>(table_ptr[j].signature));
									JVM::get().jvmti->Deallocate(reinterpret_cast<unsigned char*>(table_ptr[j].name));
									JVM::get().jvmti->Deallocate(reinterpret_cast<unsigned char*>(table_ptr[j].generic_signature));
								}

							}
						}
#endif // DEBUG
						
						std::cout << "gameRender :" << gameRender << "\np_109090_ :" << p_109090_ << "\np_109091_ :" << p_109091_ << "\np_109092_ :" << p_109092_ << "\nmatrix4f :" << matrix4f << "\n" << std::endl;
						jclass klass = JNI::get_env()->GetObjectClass(matrix4f);
						if (klass)
						{
							auto instacne = java_hotspot::instance_klass::get_instance_class(klass);
							std::cout << "matrix 4f klass:" << instacne->get_name()->to_string() << std::endl;
						}
						bp->java_thread->set_thread_state(orginal_state);
						return;
					});
				break;
			}

		}


	}

}