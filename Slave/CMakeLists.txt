add_executable( slave
				slave.c
			  )

target_link_libraries( slave
					   stdlib
					   cyw43_arch
					   multicore
					 )

example_auto_set_url( slave )