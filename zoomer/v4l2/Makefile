
CUR_SOURCE=${wildcard *.c}

CUR_OBJS=${patsubst %.c, %.o, $(CUR_SOURCE)}

all:$(CUR_OBJS)


$(CUR_OBJS):%.o:%.c
	$(CC) -c $^ -o $(TOP_DIR)/$(OBJS_DIR)/$@ $(INC_PATH)



INC_PATH :=  -I $(TOP_DIR)/include 
