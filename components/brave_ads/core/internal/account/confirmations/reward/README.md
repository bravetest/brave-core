# Reward Confirmation Token

Create an anonymous [confirmation token](https://github.com/brave/brave-browser/wiki/Security-and-privacy-model-for-ad-confirmations).

See [confirmation redemption](../../utility/redeem_confirmation/README.md).

### Payload

| key  | format  | example  | note  |
| -----------  | ------  | -------  | ----  |
| creativeInstanceId  | UUID  | e4958d00-e35c-4134-a408-1fbcf274d5ae  | An id that references the specific ad creative that the user engaged with. This will be the same for any user thatxwxzw engages with this ad.  |
| transactionId  | UUID  | c14d370c-1178-4c73-8385-1cfa17200646  | A unique id for the event that is not re-used. This cannot be linked between ad events or users.  |
| type  | - view<br>- click<br>- landed<br>- conversion<br>- media_play[^1]<br>- media_25[^1]<br>- media_100[^1]<br>- upvote<br>- downvote<br>- flag<br>- bookmark &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;| {"type":"view"}  |  |
| confirmation token  | JSON  | {"blindedPaymentTokens": ["Ev5JE4/9TZI/5TqyN9JWfJ1To0HBwQw2rWeAPcdjX3Q="], "publicKey": "RJ2i/o/pZkrH+i0aGEMY1G9FXtd7Q7gfRi3YdNRnDDk="}  |  |
| user data  | JSON  | {"buildChannel":"release","catalog":[{"id":"29e5c8bc0ba319069980bb390d8e8f9b58c05a20"}],"conversion":[{"action":"view"}],"countryCode":"US","createdAtTimestamp":"2020-11-18T12:00:00.000Z","httpResponseStatus":"errorPage","platform":"windows","rotatingHash":"I6KM54gXOrWqRHyrD518LmhePLHpIk4KSgCKOl0e3sc=","segment":"untargeted","studies":[],"topSegment":[],"versionNumber":"1.2.3.4"}  | See [README.md](../../user_data/README.md#brave-rewards-users)  |

Please add to it!

[^1]: MVP for iOS in Japan.
