# screen-nx
A screenshot uploader for the Nintendo Switch. Upload screenshots and recordings and easily share them with your friends! No Twitter or Nintendo account required!

![screen-nx](https://i.imgur.com/tr45Ekf.jpg)


### Site Configs
Site-specific configuration files can be created for uploading to other services, as well as changing the theme of the application itself. These can easily be selected in-app by pressing the X button at the main menu. Take a look at the [example configs](https://github.com/HookedBehemoth/screen-nx/tree/master/config/sites) to learn how to make them, and feel free to submit a pull request with your own creations!

tg-video and tg-photo require the user to add his own authentification.

#### How to Set up Telegram site config to send the upload to you as a DM.

PreRequsite #1: Have a telegram bot and its token.
1. Start a new chat with [@BotFather](https://t.me/BotFather)
2. Send `/newbot`
3. Tell [@BotFather](https://t.me/BotFather) what you want to name your bot. For example: "Forte's Switch"
4. Tell [@BotFather](https://t.me/BotFather) what username you want your bot to have. For Example: "ForteSwitch_bot"
5. [@BotFather](https://t.me/BotFather) will then give you the Token you'll need.
Hold onto this token, and use it when we need the `TOKEN`

PreRequsite #2: Have the chat id of where you want the telegram bot to send the file. 
1. Open `https://api.telegram.org/bot[bot-token]/getUpdates` in a new browser tab, being sure to replace the `[bot-token]` with your actual bot token
2. Send a message to your bot, any message will do. You can do this by clicking the t.me link that [@BotFather](https://t.me/BotFather) gives you after setting up a bot. 
3. Refresh the tab we opened in step 1. Look for the `chat-id` in the result > message > from > id field. For me this was an 8 digit number.
Hold onto this number, and use it when we need the `chat-id`.

Once you've got your pre-requesites handled, now we make the config file.

1. Download the `tg-photo.ini` `tg-video.ini` or `tg-document.ini` file from the [sites directory](https://github.com/HookedBehemoth/screen-nx/tree/master/config/sites). Which one you use is up to you, but if you're not sure which to use, just use the tg-document file. 
2. Open the file in your text editor of choice, Notepad++ for example
3. In the "hoster" section update the "url" field replace where it says `TOKEN` with your bot token that [@BotFather](https://t.me/BotFather) gave you. Be sure not to remove `bot` from the url. For example: `https://api.telegram.org/bot9999999999:AAAAAAAAAAAAAAAAAAAAAAAA/sendDocument`
3. In the `[1]` section, update the "data" field to be the `chat-id` we got in PreRequsite #2
4. Save the file and save it to your SD card. X:/switch/screen-nx/sites/ by default, you should see the 0x0.ini and the lewd-pics.ini file already there. 
5. Next time you open screen-nx just press the `X` button to select which config you want, and select the telegram option.
