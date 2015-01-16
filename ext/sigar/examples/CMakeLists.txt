#--------------------------------------------------------------------
# Create project
ivw_project(sigar-systeminfo)
#--------------------------------------------------------------------
# Need to add dependent directories before creating application
ivw_add_dependency_directories(Sigar)
#--------------------------------------------------------------------
# Create test
ivw_create_test(systeminfo.c)
#--------------------------------------------------------------------
# Add dependency
ivw_add_dependencies(Sigar)


#--------------------------------------------------------------------
# Create project
ivw_project(sigar-processlist)
#--------------------------------------------------------------------
# Need to add dependent directories before creating application
ivw_add_dependency_directories(Sigar)
#--------------------------------------------------------------------
# Create test
ivw_create_test(processlist.c)
#--------------------------------------------------------------------
# Add dependency
ivw_add_dependencies(Sigar)

