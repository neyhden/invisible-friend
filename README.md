# Automatic Invisible Friend program

To automatically send invisible friends via mail (works with gmail, dont know about other servers)

## Usage

Execute the program passing the config and mails file:
```
./invisible_friend.out [CONFIG] [MAILS]
```

The [CONFIG] file must follow the format:
```
sender@mail.com
{GOOGLE_APP_PASSWORD}
Mail subject
Mail body where the person to give the gift to is -> %s
```

The [MAILS] file follows the format:
```
NAME1 mail1@gmail.com
NAME2 mail2@gmail.com
NAME3 mail3@gmail.com
NAME4 mail4@gmail.com
...
```
