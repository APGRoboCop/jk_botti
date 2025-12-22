//
// JK_Botti - be more human!
//
// bot_combat.cpp
//

#ifndef _WIN32
#include <cstring>
#endif

#include <extdll.h>
#include <dllapi.h>
#include <h_export.h>
#include <meta_api.h>

#include "bot.h"
#include "bot_func.h"


#define NUM_TAGS 24
char *tag1[NUM_TAGS]={
"[", "*[", "-=","-[","-]","-}","-{","<[","<]","[-","]-","{-","}-","[[","[","{","]","}","<",">","-","|","=","+"};
char *tag2[NUM_TAGS]={
"]*", "]*", "=-","]-","[-","{-","}-","]>","[>","-]","-[","-}","-{","]]","]","}","[","{",">","<","-","|","=","+"};


int bot_chat_count;
int bot_taunt_count;
int bot_whine_count;
int bot_endgame_count;

bot_chat_t bot_chat[MAX_BOT_CHAT];
bot_chat_t bot_taunt[MAX_BOT_CHAT];
bot_chat_t bot_whine[MAX_BOT_CHAT];
bot_chat_t bot_endgame[MAX_BOT_CHAT];

int recent_bot_chat[5];
int recent_bot_taunt[5];
int recent_bot_whine[5];
int recent_bot_endgame[5];

int player_count;
char player_names[32][33];  // 32 players max, 32 chars + null

extern int bot_chat_tag_percent;
extern int bot_chat_drop_percent;
extern int bot_chat_swap_percent;
extern int bot_chat_lower_percent;


//
void LoadBotChat()
{
	char filename[256];
   char buffer[256];
	int section = -1;

	bot_chat_count = 0;
   bot_taunt_count = 0;
   bot_whine_count = 0;
   bot_endgame_count = 0;

   for (int i = 0; i < 5; i++)
   {
	  recent_bot_chat[i] = -1;
	  recent_bot_taunt[i] = -1;
	  recent_bot_whine[i] = -1;
	  recent_bot_endgame[i] = -1;
   }

   UTIL_BuildFileName_N(filename, sizeof(filename), "addons/jk_botti/jk_botti_chat.txt", nullptr);

   FILE* bfp = fopen(filename, "r");
   
   if(bfp != nullptr)
	  UTIL_ConsolePrintf("Loading %s...\n", filename);

   while (bfp != nullptr)
   {
	const char* stat = fgets(buffer, 80, bfp);

	  if (stat == nullptr)
	  {
		 fclose(bfp);
		 bfp = nullptr;
		 continue;
	  }

	  buffer[80] = 0;  // truncate lines longer than 80 characters

	  size_t length = strlen(buffer);

	  if (buffer[length-1] == '\n')
	  {
		 buffer[length-1] = 0;  // remove '\n'
		 length--;
	  }

	  if (strcmp(buffer, "[bot_chat]") == 0)
	  {
		 section = 0;
		 continue;
	  }

	  if (strcmp(buffer, "[bot_taunt]") == 0)
	  {
		 section = 1;
		 continue;
	  }

	  if (strcmp(buffer, "[bot_whine]") == 0)
	  {
		 section = 2;
		 continue;
	  }
	  
	  if (strcmp(buffer, "[bot_endgame]") == 0)
	  {
		 section = 3;
		 continue;
	  }

	  if ((length > 0) && !(buffer[0] == '!' && length==1) && (section == 0) &&  // bot chat
		  (bot_chat_count < MAX_BOT_CHAT))
	  {
		 if (buffer[0] == '!')
		 {
			safe_strcopy(bot_chat[bot_chat_count].text, sizeof(bot_chat[bot_chat_count].text), &buffer[1]);
			bot_chat[bot_chat_count].can_modify = FALSE;
		 }
		 else
		 {
			safe_strcopy(bot_chat[bot_chat_count].text, sizeof(bot_chat[bot_chat_count].text), buffer);
			bot_chat[bot_chat_count].can_modify = TRUE;
		 }

		 bot_chat_count++;
	  }

	  if ((length > 0) && !(buffer[0] == '!' && length==1) && (section == 1) &&  // bot taunt
		  (bot_taunt_count < MAX_BOT_CHAT))
	  {
		 if (buffer[0] == '!')
		 {
			safe_strcopy(bot_taunt[bot_taunt_count].text, sizeof(bot_taunt[bot_taunt_count].text), &buffer[1]);
			bot_taunt[bot_taunt_count].can_modify = FALSE;
		 }
		 else
		 {
			safe_strcopy(bot_taunt[bot_taunt_count].text, sizeof(bot_taunt[bot_taunt_count].text), buffer);
			bot_taunt[bot_taunt_count].can_modify = TRUE;
		 }

		 bot_taunt_count++;
	  }

	  if ((length > 0) && !(buffer[0] == '!' && length==1) && (section == 2) &&  // bot whine
		  (bot_whine_count < MAX_BOT_CHAT))
	  {
		 if (buffer[0] == '!')
		 {
			safe_strcopy(bot_whine[bot_whine_count].text, sizeof(bot_whine[bot_whine_count].text), &buffer[1]);
			bot_whine[bot_whine_count].can_modify = FALSE;
		 }
		 else
		 {
			safe_strcopy(bot_whine[bot_whine_count].text, sizeof(bot_whine[bot_whine_count].text), buffer);
			bot_whine[bot_whine_count].can_modify = TRUE;
		 }

		 bot_whine_count++;
	  }

	  if ((length > 0) && !(buffer[0] == '!' && length==1) && (section == 3) &&  // bot endgame
		  (bot_endgame_count < MAX_BOT_CHAT))
	  {
		 if (buffer[0] == '!')
		 {
			safe_strcopy(bot_endgame[bot_endgame_count].text, sizeof(bot_endgame[bot_endgame_count].text), &buffer[1]);
			bot_endgame[bot_endgame_count].can_modify = FALSE;
		 }
		 else
		 {
			safe_strcopy(bot_endgame[bot_endgame_count].text, sizeof(bot_endgame[bot_endgame_count].text), buffer);
			bot_endgame[bot_endgame_count].can_modify = TRUE;
		 }

		 bot_endgame_count++;
	  }
   }
}


static void BotTrimBlanks(const char *in_string, char *out_string, int sizeof_out_string)
{
	int pos = 0;
   while ((pos < sizeof_out_string) && (in_string[pos] == ' '))  // skip leading blanks
	  pos++;

   char* dest = &out_string[0];

   while ((pos < sizeof_out_string) && (in_string[pos]))
   {
	  *dest++ = in_string[pos];
	  pos++;
   }
   *dest = 0;  // store the null

   size_t i = strlen(out_string) - 1;
   while ((i > 0) && (out_string[i] == ' '))  // remove trailing blanks
   {
	  out_string[i] = 0;
	  i--;
   }
}


static int BotChatTrimTag(const char *original_name, char *out_name, int sizeof_out_name)
{
	char *pos2;
   char in_name[80];
   int result = 0;

   safe_strcopy(in_name, sizeof(in_name), original_name);

   for (int i = 0; i < NUM_TAGS; i++)
   {
	  char* pos1 = strstr(in_name, tag1[i]);
	  if (pos1)
		 pos2=strstr(pos1+strlen(tag1[i]), tag2[i]);
	  else
		 pos2 = nullptr;

	  if (pos1 && pos2 && pos1 < pos2)
	  {
		  const char* src = pos2 + strlen(tag2[i]);
		 char* dest = pos1;
		 while (*src)
			*dest++ = *src++;
		 *dest = *src;  // copy the null;

		 result = 1;
	  }
   }

   safe_strcopy(out_name, sizeof_out_name, in_name);

   BotTrimBlanks(out_name, in_name, sizeof(in_name));

   if (strlen(in_name) == 0)  // is name just a tag?
   {
	  safe_strcopy(in_name, sizeof(in_name), original_name);
	  
	  /*
	  // strip just the tag part...
	  for (i=0; i < NUM_TAGS; i++)
	  {
		 pos1=strstr(in_name, tag1[i]);
		 if (pos1)
			pos2=strstr(pos1+strlen(tag1[i]), tag2[i]);
		 else
			pos2 = NULL;

		 if (pos1 && pos2 && pos1 < pos2)
		 {
			src = pos1 + strlen(tag1[i]);
			dest = pos1;
			while (*src)
			   *dest++ = *src++;
			*dest = *src;  // copy the null;

			src = pos2 - strlen(tag2[i]);
			*src = 0; // null out the rest of the string
		 }
	  }
	  */
	  result = 0;
   }

   BotTrimBlanks(in_name, out_name, sizeof_out_name);

   out_name[sizeof_out_name-1] = 0;

   return (result);
}


static void BotDropCharacter(const char *in_string, char *out_string, int sizeof_out_string)
{
	int count;

	safe_strcopy(out_string, sizeof_out_string, in_string);

	const size_t len = strlen(out_string);
   if(len < 2)
	  return;
   
   int pos = RANDOM_LONG2(1, len - 1);  // don't drop position zero

   qboolean is_bad = !isalpha(out_string[pos]) || (out_string[pos - 1] == '%');
   for(count = 0; is_bad && count < len && count < 20; count++)
   {
	  pos = RANDOM_LONG2(1, len-1);
	  is_bad = !isalpha(out_string[pos]) || (out_string[pos-1] == '%');
   }

   if (count < len && count < 20)
   {
	   const char* src = &out_string[pos + 1];
	  char* dest = &out_string[pos];
	  while (*src)
		 *dest++ = *src++;
	  *dest = *src;  // copy the null;
   }
}


static void BotSwapCharacter(const char *in_string, char *out_string, int sizeof_out_string)
{
	int count;

	safe_strcopy(out_string, sizeof_out_string, in_string);

	const size_t len = strlen(out_string);
   if(len < 3) // must be 3, 1+1 for swap + zero must now swap = 3
	  return;
   
   int pos = RANDOM_LONG2(1, len - 2);  // don't swap position zero

   qboolean is_bad = !isalpha(out_string[pos]) || !isalpha(out_string[pos + 1]) || (out_string[pos - 1] == '%');
   for(count = 0; is_bad && count < len && count < 20; count++)
   {
	  pos = RANDOM_LONG2(1, len-2);
	  is_bad = !isalpha(out_string[pos]) || !isalpha(out_string[pos+1]) || (out_string[pos-1] == '%');
   }

   if (count < 20)
   {
	   const char temp = out_string[pos];
	  out_string[pos] = out_string[pos+1];
	  out_string[pos+1] = temp;
   }
}


static void BotChatName(const char *original_name, char *out_name, int sizeof_out_name)
{
	//always remove [lvlX] tag
   if(strncmp(original_name, "[lvl", 4) == 0 && original_name[4] >= '0' && original_name[4] <= '5' && original_name[5] == ']')
   {
	   char temp_lvlXless_name[80];
	   safe_strcopy(temp_lvlXless_name, sizeof(temp_lvlXless_name), &original_name[6]);
	  original_name = temp_lvlXless_name;
   }

   if (RANDOM_LONG2(1, 100) <= bot_chat_tag_percent)
   {
	  char temp_name[80];

	  safe_strcopy(temp_name, sizeof(temp_name), original_name);

	  while (BotChatTrimTag(temp_name, out_name, sizeof_out_name))
	  {
		 safe_strcopy(temp_name, sizeof(temp_name), out_name);
	  }
   }
   else
   {
	  safe_strcopy(out_name, sizeof_out_name, original_name);
   }

   if (RANDOM_LONG2(1, 100) <= bot_chat_lower_percent)
   {
	  int pos = 0;
	  while ((pos < sizeof_out_name) && (out_name[pos]))
	  {
		 out_name[pos] = tolower(out_name[pos]);
		 pos++;
	  }
   }
}


static void BotChatText(const char *in_text, char *out_text, int sizeof_out_text)
{
	char temp_text[81];
   int count;

   safe_strcopy(temp_text, sizeof(temp_text), in_text);

   if (RANDOM_LONG2(1, 100) <= bot_chat_drop_percent)
   {
	  count = RANDOM_LONG2(1, 3);

	  while (count)
	  {
		 BotDropCharacter(temp_text, out_text, sizeof_out_text);
		 safe_strcopy(temp_text, sizeof(temp_text), out_text);
		 count--;
	  }
   }

   if (RANDOM_LONG2(1, 100) <= bot_chat_swap_percent)
   {
	  count = RANDOM_LONG2(1, 2);

	  while (count)
	  {
		 BotSwapCharacter(temp_text, out_text, sizeof_out_text);
		 safe_strcopy(temp_text, sizeof(temp_text), out_text);
		 count--;
	  }
   }

   if (RANDOM_LONG2(1, 100) <= bot_chat_lower_percent)
   {
	  int pos = 0;
	  while (temp_text[pos])
	  {
		 temp_text[pos] = tolower(temp_text[pos]);
		 pos++;
	  }
   }

   safe_strcopy(out_text, sizeof_out_text, temp_text);
}


static void BotChatGetPlayers()
{
	player_count = 0;

   for (int index = 1; index <= gpGlobals->maxClients; index++)
   {
	  const edict_t *pPlayer = INDEXENT(index);

	  // skip invalid players
	  if ((pPlayer) && (!pPlayer->free) && !FBitSet(pPlayer->v.flags, FL_PROXY))
	  {
		 if (pPlayer->v.netname)
		 {
			const char* pName = STRING(pPlayer->v.netname);

			if (*pName != 0)
			{
			   safe_strcopy(player_names[player_count], sizeof(player_names[player_count]), pName);

			   player_count++;
			}
		 }
	  }
   }
}


static void BotChatFillInName(char *bot_say_msg, int sizeof_msg, const char *chat_text, const char *chat_name, const char *bot_name)
{
	const size_t clen = strlen(chat_text);
   int i = 0;
   int o = 0;

   while(i < clen && o < sizeof_msg)
   {
	  if(chat_text[i] == '%')
	  {
		 if(i + 1 < clen)
		 {
			if(chat_text[i + 1] == 'n' || chat_text[i + 1] == 'r')
			{
			   const char * to_output = chat_name;
			   
			   if(chat_text[i + 1] == 'r')
			   {
				   char random_name[64];
				   BotChatGetPlayers();
		 
				  // pick a name at random from the list of players...
				  int index = RANDOM_LONG2(0, player_count-1);
				  int count = 0;
				  
				  bool is_bad = (strcmp(player_names[index], chat_name) == 0) ||
								(strcmp(player_names[index], bot_name) == 0);

				  while ((is_bad) && (count < 20))
				  {
					 index = RANDOM_LONG2(0, player_count-1);

					 is_bad = (strcmp(player_names[index], chat_name) == 0) ||
							  (strcmp(player_names[index], bot_name) == 0);
					 
					 count++;
				  }

				  BotChatName(player_names[index], random_name, sizeof(random_name));
				  
				  to_output = random_name;
			   }
			   
			   // copy chat name to output
			   const size_t nlen = strlen(to_output);
			   int n = 0;
			   
			   while(n < nlen && o < sizeof_msg)
				  bot_say_msg[o++] = to_output[n++];
			   
			   // skip %X
			   i+=2;
			   continue;
			}
		 }
	  }
	  
	  bot_say_msg[o++] = chat_text[i++];
   }
   
   if(o < sizeof_msg)
	  bot_say_msg[o] = 0;
   else
	  bot_say_msg[sizeof_msg - 1] = 0;
}


// taunt on killed player
void BotChatTaunt(bot_t& pBot, const edict_t* victim_edict)
{
	if(pBot.b_bot_say && pBot.f_bot_say >= gpGlobals->time)
	  return;
   
   // are there any taunt messages and should the bot taunt?
   if ((bot_taunt_count > 0) &&
	   (RANDOM_LONG2(1,100) <= pBot.taunt_percent))
   {
	   char chat_name[64];
	   char chat_text[81];
	   int taunt_index = 0;
	  int i;

	  int recent_count = 0;

	  while (recent_count < 5)
	  {
		 taunt_index = RANDOM_LONG2(0, bot_taunt_count-1);

		 qboolean used = FALSE;

		 for (i=0; i < 5; i++)
		 {
			if (recent_bot_taunt[i] == taunt_index)
			   used = TRUE;
		 }

		 if (used)
			recent_count++;
		 else
			break;
	  }

	  for (i=4; i > 0; i--)
		 recent_bot_taunt[i] = recent_bot_taunt[i-1];

	  recent_bot_taunt[0] = taunt_index;

	  if (bot_taunt[taunt_index].can_modify)
		 BotChatText(bot_taunt[taunt_index].text, chat_text, sizeof(chat_text));
	  else
		 safe_strcopy(chat_text, sizeof(chat_text), bot_taunt[taunt_index].text);

	  if (victim_edict->v.netname)
	  {
		  char temp_name[64];
		  safe_strcopy(temp_name, sizeof(temp_name), STRING(victim_edict->v.netname));

		 BotChatName(temp_name, chat_name, sizeof(chat_name));
	  }
	  else
		 strcpy(chat_name, "NULL");

	  const char* bot_name = STRING(pBot.pEdict->v.netname);

	  BotChatFillInName(pBot.bot_say_msg, sizeof(pBot.bot_say_msg), chat_text, chat_name, bot_name);

	  // set chat flag and time to chat (typing one character takes 0.2sec) ...
	  pBot.b_bot_say = TRUE;
	  pBot.f_bot_say = gpGlobals->time + RANDOM_FLOAT2(0.5f, 1.0f) + strlen(pBot.bot_say_msg) * RANDOM_FLOAT2(0.2f, 0.3f);
   }
}


// did another player kill this bot AND bot whine messages loaded AND
void BotChatWhine(bot_t &pBot)
{
	if(pBot.b_bot_say && pBot.f_bot_say >= gpGlobals->time)
	  return;

   const edict_t *pEdict = pBot.pEdict;
   
   // has the bot been alive for at least 15 seconds AND
   if ((pBot.killer_edict != nullptr) && (bot_whine_count > 0) &&
	   ((pBot.f_bot_spawn_time + 15.0f) <= gpGlobals->time))
   {
	   if ((RANDOM_LONG2(1,100) <= pBot.whine_percent))
	   {
		  int i;
		  int whine_index = 0;
		  char chat_name[64];
		  char chat_text[81];
		  int recent_count = 0;

		 while (recent_count < 5)
		 {
			whine_index = RANDOM_LONG2(0, bot_whine_count-1);

			qboolean used = FALSE;

			for (i=0; i < 5; i++)
			{
			   if (recent_bot_whine[i] == whine_index)
				  used = TRUE;
			}

			if (used)
			   recent_count++;
			else
			   break;
		 }

		 for (i=4; i > 0; i--)
			recent_bot_whine[i] = recent_bot_whine[i-1];

		 recent_bot_whine[0] = whine_index;

		 if (bot_whine[whine_index].can_modify)
			BotChatText(bot_whine[whine_index].text, chat_text, sizeof(chat_text));
		 else
			safe_strcopy(chat_text, sizeof(chat_text), bot_whine[whine_index].text);

		 if (pBot.killer_edict->v.netname)
		 {
			 char temp_name[64];
			 safe_strcopy(temp_name, sizeof(temp_name), STRING(pBot.killer_edict->v.netname));

			BotChatName(temp_name, chat_name, sizeof(chat_name));
		 }
		 else
			strcpy(chat_name, "NULL");

		 const char* bot_name = STRING(pEdict->v.netname);

		 BotChatFillInName(pBot.bot_say_msg, sizeof(pBot.bot_say_msg), chat_text, chat_name, bot_name);

		 // set chat flag and time to chat (typing one character takes 0.2sec) ...
		 pBot.b_bot_say = TRUE;
		 pBot.f_bot_say = gpGlobals->time + RANDOM_FLOAT2(0.5f, 1.0f) + strlen(pBot.bot_say_msg) * RANDOM_FLOAT2(0.2f, 0.3f);
	  }
   }
}


// just say something
void BotChatTalk(bot_t &pBot)
{
	if(pBot.b_bot_say && pBot.f_bot_say >= gpGlobals->time)
	  return;

   const edict_t *pEdict = pBot.pEdict;
   
   if ((bot_chat_count > 0) && (pBot.f_bot_chat_time < gpGlobals->time))
   {
	  pBot.f_bot_chat_time = gpGlobals->time + 30.0f;

	  if (RANDOM_LONG2(1,100) <= pBot.chat_percent)
	  {
		  char chat_name[64];
		  char chat_text[81];
		  int chat_index = 0;
		 int i;

		 int recent_count = 0;

		 while (recent_count < 5)
		 {
			chat_index = RANDOM_LONG2(0, bot_chat_count-1);

			qboolean used = FALSE;

			for (i=0; i < 5; i++)
			{
			   if (recent_bot_chat[i] == chat_index)
				  used = TRUE;
			}

			if (used)
			   recent_count++;
			else
			   break;
		 }

		 for (i=4; i > 0; i--)
			recent_bot_chat[i] = recent_bot_chat[i-1];

		 recent_bot_chat[0] = chat_index;

		 if (bot_chat[chat_index].can_modify)
			BotChatText(bot_chat[chat_index].text, chat_text, sizeof(chat_text));
		 else
			safe_strcopy(chat_text, sizeof(chat_text), bot_chat[chat_index].text);

		 safe_strcopy(chat_name, sizeof(chat_name), STRING(pBot.pEdict->v.netname));

		 const char* bot_name = STRING(pEdict->v.netname);

		 BotChatFillInName(pBot.bot_say_msg, sizeof(pBot.bot_say_msg), chat_text, chat_name, bot_name);

		 // set chat flag and time to chat (typing one character takes 0.2sec) ...
		 pBot.b_bot_say = TRUE;
		 pBot.f_bot_say = gpGlobals->time + RANDOM_FLOAT2(0.5f, 1.0f) + strlen(pBot.bot_say_msg) * RANDOM_FLOAT2(0.2f, 0.3f);
	  }
   }
}


// endgame say
void BotChatEndGame(bot_t &pBot)
{
	const edict_t *pEdict = pBot.pEdict;
   
   if ((bot_endgame_count > 0) && RANDOM_LONG2(1,100) <= pBot.endgame_percent)
   {
	   char chat_name[64];
	   char chat_text[81];
	   int endgame_index = 0;
	  int i;
	  
	  int recent_count = 0;

	  while (recent_count < 5)
	  {
		 endgame_index = RANDOM_LONG2(0, bot_endgame_count-1);

		 qboolean used = FALSE;

		 for (i=0; i < 5; i++)
		 {
			if (recent_bot_endgame[i] == endgame_index)
			   used = TRUE;
		 }

		 if (used)
			recent_count++;
		 else
			break;
	  }

	  for (i=4; i > 0; i--)
		 recent_bot_endgame[i] = recent_bot_endgame[i-1];

	  recent_bot_endgame[0] = endgame_index;

	  if (bot_endgame[endgame_index].can_modify)
		 BotChatText(bot_endgame[endgame_index].text, chat_text, sizeof(chat_text));
	  else
		 safe_strcopy(chat_text, sizeof(chat_text), bot_endgame[endgame_index].text);

	  safe_strcopy(chat_name, sizeof(chat_name), STRING(pBot.pEdict->v.netname));

	  const char* bot_name = STRING(pEdict->v.netname);

	  BotChatFillInName(pBot.bot_say_msg, sizeof(pBot.bot_say_msg), chat_text, chat_name, bot_name);

	  // set chat flag and time to chat (typing one character takes 0.2sec) ...
	  pBot.b_bot_say = TRUE;
	  pBot.f_bot_say = gpGlobals->time + RANDOM_FLOAT2(0.3f, 2.0f) + strlen(pBot.bot_say_msg) * RANDOM_FLOAT2(0.2f, 0.3f);
   }
}

