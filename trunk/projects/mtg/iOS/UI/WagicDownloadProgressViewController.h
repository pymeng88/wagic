//
//  WagicDownloadProgressView.h
//  wagic
//
//  Created by Michael Nguyen on 12/10/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface WagicDownloadProgressViewController : UIViewController<UIAlertViewDelegate>
{
 
}

@property (nonatomic, retain) UITextView     *downloadMessageStatus;
@property (nonatomic, retain) UIProgressView *downloadProgressView;

@end
