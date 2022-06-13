import cv2
import mediapipe as mp
import time
from SM import StateMachine

class handDetector():

    def __init__(self, mode=False, maxHands=2, detectionCon=0.5, trackCon=0.5):
        self.mode = mode
        self.maxHands = maxHands
        self.detectionCon = detectionCon
        self.trackCon = trackCon
        self.mpHands = mp.solutions.hands
        self.hands = self.mpHands.Hands(self.mode, self.maxHands,
                                        self.detectionCon, self.trackCon)
        self.mpDraw = mp.solutions.drawing_utils

    def findHands(self, img, draw=True):
        imgRGB = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        self.results = self.hands.process(imgRGB)
        # print(results.multi_hand_landmarks)
        if self.results.multi_hand_landmarks:
            for handLms in self.results.multi_hand_landmarks:
                if draw:
                    self.mpDraw.draw_landmarks(img, handLms,
                                               self.mpHands.HAND_CONNECTIONS)
        return img
    def findPosition(self, img, handNo=0, draw=True):
        lmList = []
        if self.results.multi_hand_landmarks:
            myHand = self.results.multi_hand_landmarks[handNo]
            for id, lm in enumerate(myHand.landmark):
                # print(id, lm)
                h, w, c = img.shape
                cx, cy = int(lm.x * w), int(lm.y * h)
                # print(id, cx, cy)
                lmList.append([id, cx, cy])
                if draw:
                    cv2.circle(img, (cx, cy), 15, (255, 0, 255), cv2.FILLED)
        return lmList

def hasSign(lmList, currentSign, lastSign, lastSignCounter):
    dedo4x = lmList[4][1]
    dedo8x = lmList[8][1]
    dedo12x = lmList[12][1]
    dedo16x = lmList[16][1]
    dedo20x = lmList[20][1]
    dedo4y = lmList[4][2]
    dedo8y = lmList[8][2]
    dedo12y = lmList[12][2]
    dedo16y = lmList[16][2]
    dedo20y = lmList[20][2]

    #print(dedo4,dedo8,dedo12,dedo16,dedo20)
    if dedo4y < dedo8y and dedo4y < dedo12y and dedo4y < dedo16y and dedo4y < dedo20y and xAligned(lmList):
        readSign = (2, 'Cool Sign')
    elif dedo4y > dedo8y and dedo4y > dedo12y and dedo4y > dedo16y and dedo4y > dedo20y and xAligned(lmList):
        readSign = (1, 'Not Cool Sign')
    else:
        readSign = (0, 'Neutral')

    if lastSignCounter == 10:
        lastSignCounter = 0
        currentSign = readSign
    elif lastSign != readSign:
        lastSignCounter = 1
    else:
        lastSignCounter += 1

    lastSign = readSign

    return (currentSign, lastSign, lastSignCounter)

def xAligned(lmList):
    dedo4x = lmList[4][1]
    dedo8x = lmList[8][1]
    dedo12x = lmList[12][1]
    dedo16x = lmList[16][1]
    dedo20x = lmList[20][1]
    leftLimit = dedo4x - 100
    rightLimit = dedo4x + 100

    if ((dedo8x > leftLimit and dedo8x < rightLimit) and (dedo12x > leftLimit and dedo12x < rightLimit)
    and (dedo16x > leftLimit and dedo16x < rightLimit) and (dedo20x > leftLimit and dedo20x < rightLimit)):
        return True
    else:
        return False

def main():
    pTime = 0
    cTime = 0
    cap = cv2.VideoCapture('videos/hand.mp4')
    dispW = 640
    dispH = 480
    font=cv2.FONT_HERSHEY_SIMPLEX
    cap.set(cv2.CAP_PROP_FRAME_WIDTH,dispW)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT,dispH)

    sm = init_sm()

    currentSign = (0, 'Neutral')
    lastSign = (0, 'Neutral')
    lastSignCounter = 10

    detector = handDetector()
    while True:
        success, img = cap.read()
        if not success:
            print('Video Ended')
            break
        img = detector.findHands(img)
        lmList = detector.findPosition(img)
        if len(lmList) != 0:
            currentSign, lastSign, lastSignCounter = hasSign(lmList, currentSign, lastSign, lastSignCounter)

            cv2.putText(img, currentSign[1], (80, 70), cv2.FONT_HERSHEY_PLAIN, 3,
                    (255, 0, 0), 3)

            sm.process_event(str(currentSign[0]))

        cTime = time.time()
        fps = 1 / (cTime - pTime)
        pTime = cTime
        cv2.putText(img, str(int(fps)), (10, 70), cv2.FONT_HERSHEY_PLAIN, 3,
                    (255, 0, 255), 3)

        cv2.imshow("Image", img)
        if cv2.waitKey(1)==ord('q'):
             break
    cap.release()
    sm.close()

def init_sm ():
    sm = StateMachine()

    sm.add_on_entry("0", lambda: print("Entering Neutral"))
    sm.add_on_entry("1", lambda: print("Entering Not Cool Sign"))
    sm.add_on_entry("2", lambda: print("Entering Cool Sign"))

    sm.add_on_leave("0", lambda: print("Leaving Neutral"))
    sm.add_on_leave("1", lambda: print("Leaving Not Cool Sign"))
    sm.add_on_leave("2", lambda: print("Leaving Cool Sign"))

    sm.add_transition("0", "1", lambda: print("Going from Neutral to Not Cool Sign"))
    sm.add_transition("0", "2", lambda: print("Going from Neutral to Cool Sign"))
    sm.add_transition("1", "2", lambda: print("Going from Not Cool Sign to Cool Sign"))
    sm.add_transition("2", "1", lambda: print("Going from Cool Sign to Not Cool Sign"))
    sm.add_transition("1", "0", lambda: print("Going from Not Cool Sign to Neutral"))
    sm.add_transition("2", "0", lambda: print("Going from Cool Sign to Neutral"))

    return sm

if __name__ == "__main__":
    main()
    cv2.destroyAllWindows()
