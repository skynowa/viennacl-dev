#ifndef VIENNACL_GENERATOR_CUSTOM_OPERATION_HPP
#define VIENNACL_GENERATOR_CUSTOM_OPERATION_HPP

#include "viennacl/generator/code_generation/frontend.hpp"
#include "viennacl/generator/symbolic_types.hpp"
#include "viennacl/generator/dummy_types.hpp"
#include "viennacl/tools/shared_ptr.hpp"
#include <bitset>

namespace viennacl
{
  namespace generator
  {


  /** @brief A class for making a custom operation */
      class custom_operation
      {

      private:
          void compile_program(std::string const & pgm_name) const{
//              std::cout << source_code_ << std::endl;
              assert(!source_code_.empty() && " Custom Operation not initialized ");
              viennacl::ocl::program& program = viennacl::ocl::current_context().add_program(source_code_, pgm_name);
              for(std::map<std::string, generator::code_generation::kernel_infos_t>::const_iterator it = kernels_infos_.begin() ; it !=kernels_infos_.end() ; ++it){
                  program.add_kernel(it->first);
              }
          }

          void init() {
              if(source_code_.empty()) source_code_ = operations_manager_.get_source_code(kernels_infos_);
          }


      public :

          custom_operation(){ }

          template<class T0>
          custom_operation(T0 const & op0){
              add(op0);
          }

          template<class T>
          void add(T const & op){
              T copy(op);
              copy.bind(shared_infos_,temporaries_);
              operations_manager_.add(copy);
          }

          std::list<code_generation::kernel_infos_t> kernels_list(){
              return operations_manager_.get_kernels_list();
          }

          code_generation::operations_manager & operations_manager(){
              return operations_manager_;
          }



          viennacl::ocl::program & program(){
              init();
              std::string program_name_ = operations_manager_.repr();
              if(!viennacl::ocl::current_context().has_program(program_name_)){
                  compile_program(program_name_);
              }
              return viennacl::ocl::current_context().get_program(program_name_);
          }

          void execute(){
              viennacl::ocl::program & pgm = program();
              for(std::map<std::string, generator::code_generation::kernel_infos_t>::iterator it = kernels_infos_.begin() ; it != kernels_infos_.end() ; ++it){
                  viennacl::ocl::kernel& k = pgm.get_kernel(it->first);
                  set_arguments(k,it->second.arguments());
                  it->second.config_nd_range(k);
                  viennacl::ocl::enqueue(k);
              }
          }


          std::string source_code() const{
              return source_code_;
          }

        private:
          code_generation::operations_manager operations_manager_;
          shared_infos_map_t shared_infos_;
          temporaries_map_t temporaries_;
          std::map<std::string, generator::code_generation::kernel_infos_t> kernels_infos_;
          std::string source_code_;
          std::string program_name_;

     };
  }
}
#endif // CUSTOM_OPERATION_HPP
