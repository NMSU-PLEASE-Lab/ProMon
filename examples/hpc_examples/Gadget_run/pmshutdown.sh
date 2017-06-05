# Allow events at server to finish? (I think I am just too cautious here)
sleep 4

echo "Application is done...now killing Promon server"
PMPID=`ps ax | grep promon_an | grep -v grep | awk '{print $1}'`
if [ "z$PMPID" = "z" ]; then
    echo "No server running!?!"
else
    kill $PMPID
    echo "Server $PMPID killed"
fi
