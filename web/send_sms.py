import africastalking

# TODO: Initialize Africa's Talking

africastalking.initialize(
    username='abayosincere11',
    api_key='atsk_74bffd1c1dfd89ea85d0da02a2d2b5018141a77f7fbaa08afa92573e5a75eaa476989cbc'
)

sms = africastalking.SMS

class send_sms():

    def sending(self):
            # Set the numbers in international format
            recipients = ["+250786729283"]
            # Set your message
            message = "Hello World, am testing sms!"
            # Set your shortCode or senderId
            sender = "CodeFusion"
            try:
                response = self.sms.send(message, recipients, sender)
                print (response)
            except Exception as e:
                print (f'Houston, we have a problem: {e}')
