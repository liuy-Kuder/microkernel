#include "command.h"

struct list_head __command_list = {
	.next = &__command_list,
	.prev = &__command_list,
};

struct command_t * search_command(const char * name)
{
	struct command_t * pos, * n;

	if(!name)
		return NULL;

	list_for_each_entry_safe(pos, n,struct command_t, &__command_list, list)
	{
		if(strcmp(pos->name, name) == 0)
			return pos;
	}
	return NULL;
}

uint8_t register_command(struct command_t * cmd)
{
	if(!cmd || !cmd->name || !cmd->exec)
		return FALSE;

	if(search_command(cmd->name))
		return FALSE;

	spin_lock_irq();
	list_add_tail(&cmd->list, &__command_list);
	spin_unlock_irq();
	return TRUE;
}

uint8_t unregister_command(struct command_t * cmd)
{
	if(!cmd || !cmd->name)
		return FALSE;
	spin_lock_irq();
	list_del(&cmd->list);
	spin_unlock_irq();
	return TRUE;
}
