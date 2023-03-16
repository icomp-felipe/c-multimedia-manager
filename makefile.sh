#/bin/sh!

command="gcc -pipe -Llibconfuse -Llibprogressbar -Wl,-rpath=libconfuse,-rpath=libprogressbar -Wall -o media indexManager.c indexManager.h libmedia.h media.c media.h dependencyLoader.c dependencyLoader.h fileOperations.c fileOperations.h mediaTools.c mediaTools.h passwordManager.c passwordManager.h propertiesManager.c propertiesManager.h supportedMedia.c supportedMedia.h autoDelete.c autoDelete.h commons.c commons.h fileUtils.c fileUtils.h stringUtils.c stringUtils.h mediaConverter.c mediaConverter.h databaseDriver.c databaseDriver.h logger.c logger.h archiver.c archiver.h userGUI.c userGUI.h -lconfuse -lpthread -lprogressbar -lncurses"

echo "===> Gererating binary executable file..."
echo "$command"
$command

echo ":: done"