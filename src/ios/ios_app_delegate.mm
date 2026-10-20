#ifndef IO_UNDER_QT
#import "UIKit/UIKit.h"
#import "UserNotifications/UserNotifications.h"
#import <objc/runtime.h>
#include <QDebug>
#include <QMetaObject>
#include "homeform.h"

// Qt defines QIOSApplicationDelegate internally as a UIResponder-backed
// UIApplicationDelegate.  Keep the local declaration aligned with that shape
// so category methods can legally forward unhandled events to super.
@interface QIOSApplicationDelegate : UIResponder <UIApplicationDelegate>
@end

@interface QIOSApplicationDelegate (QZApplicationDelegate)
@end

@interface UIApplication (QZKeyboardShortcuts)
- (void)qz_sendEvent:(UIEvent *)event;
@end

@implementation UIApplication (QZKeyboardShortcuts)

+ (void)load {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        Method originalMethod = class_getInstanceMethod(self, @selector(sendEvent:));
        Method swizzledMethod = class_getInstanceMethod(self, @selector(qz_sendEvent:));
        method_exchangeImplementations(originalMethod, swizzledMethod);
    });
}

- (void)qz_sendEvent:(UIEvent *)event {
    bool didHandleShortcut = false;

    if (@available(iOS 13.4, *)) {
        if ([event isKindOfClass:[UIPressesEvent class]]) {
            UIPressesEvent *pressesEvent = (UIPressesEvent *)event;
            for (UIPress *press in pressesEvent.allPresses) {
                if (press.phase != UIPressPhaseBegan) {
                    continue;
                }

                UIKey *key = press.key;
                if (key == nil || key.charactersIgnoringModifiers.length == 0) {
                    continue;
                }

                const QString sequence =
                    QString::fromUtf8(key.charactersIgnoringModifiers.UTF8String).trimmed().toUpper();
                if (homeform::singleton() && homeform::singleton()->handleKeyboardShortcut(sequence)) {
                    didHandleShortcut = true;
                }
            }
        }
    }

    if (!didHandleShortcut) {
        [self qz_sendEvent:event];
    }
}

@end

@implementation QIOSApplicationDelegate (QZApplicationDelegate)

- (BOOL)application:(UIApplication *)application
    didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    qDebug() << "QZ iOS launch";
    UNUserNotificationCenter* center = [UNUserNotificationCenter currentNotificationCenter];
    [center requestAuthorizationWithOptions:UNAuthorizationOptionBadge
      completionHandler:^(BOOL granted, NSError *error){
          if(granted == YES) {
              [[UIApplication sharedApplication] setMinimumBackgroundFetchInterval:UIApplicationBackgroundFetchIntervalMinimum];
          };
      }];

    return YES;
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    Q_UNUSED(application)
    qDebug() << "QZ iOS lifecycle: applicationWillEnterForeground";
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    Q_UNUSED(application)
    qDebug() << "QZ iOS lifecycle: applicationDidBecomeActive";
}

- (void)pressesBegan:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event {
    bool didHandleShortcut = false;

    if (@available(iOS 13.4, *)) {
        for (UIPress *press in presses) {
            UIKey *key = press.key;
            if (key == nil || key.charactersIgnoringModifiers.length == 0) {
                continue;
            }

            const QString sequence =
                QString::fromUtf8(key.charactersIgnoringModifiers.UTF8String).trimmed().toUpper();
            if (homeform::singleton() && homeform::singleton()->handleKeyboardShortcut(sequence)) {
                didHandleShortcut = true;
            }
        }
    }

    if (!didHandleShortcut) {
        [super pressesBegan:presses withEvent:event];
    }
}

- (void)application:(UIApplication *)application
performFetchWithCompletionHandler:(void (^)(UIBackgroundFetchResult result))completionHandler
{
}

@end
#endif
