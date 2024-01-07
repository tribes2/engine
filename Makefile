
#--------------------------------------
# include and verify the users mk/conf.mk

-include ../mk/conf.mk

ifndef CONFIG_STATUS
doConfigure: 
	$(warning Configuration file not defined)
	#@make --no-print-directory -f ../mk/configure.mk
else
ifeq ($(CONFIG_STATUS),INVALID)
doConfigure:
	$(warning Invalid Configuration file)
	#@make --no-print-directory -f mk/configure.mk
else
	include ../mk/conf.$(COMPILER).$(OS).mk
	include ../mk/conf.$(COMPILER).mk
endif
endif


include targets.v12.mk

include ../mk/conf.common.mk


#default:
#	echo default.

ifneq ($(MAKECMDGOALS),clean)
-include $(addprefix $(DIR.OBJ)/, $(addsuffix $(DEP), $(basename $(filter %.cc %.c,$(SOURCES)))))
endif

