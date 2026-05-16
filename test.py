import cv2
import numpy as np
import mediapipe as mp
import tflite_runtime.interpreter as tflite

# =========================
# LOAD TFLITE MODELS
# =========================

eye_interpreter = tflite.Interpreter(
    model_path="eye_model.tflite"
)

mouth_interpreter = tflite.Interpreter(
    model_path="mouth_model.tflite"
)

eye_interpreter.allocate_tensors()
mouth_interpreter.allocate_tensors()

# Input / output details

eye_input = eye_interpreter.get_input_details()
eye_output = eye_interpreter.get_output_details()

mouth_input = mouth_interpreter.get_input_details()
mouth_output = mouth_interpreter.get_output_details()

# =========================
# MEDIAPIPE FACEMESH
# =========================

mp_face_mesh = mp.solutions.face_mesh

face_mesh = mp_face_mesh.FaceMesh(
    static_image_mode=False,
    max_num_faces=1,
    refine_landmarks=True
)

LEFT_EYE = [33, 133, 160, 159, 158, 144, 145, 153]
RIGHT_EYE = [362, 263, 387, 386, 385, 373, 374, 380]

MOUTH = [61, 291, 13, 14]

# =========================
# PREPROCESS
# =========================

def preprocess_crop(frame, x1, y1, x2, y2):

    h, w = frame.shape[:2]

    x1 = max(0, x1)
    y1 = max(0, y1)
    x2 = min(w, x2)
    y2 = min(h, y2)

    crop = frame[y1:y2, x1:x2]

    if crop.size == 0:
        return None

    crop = cv2.cvtColor(crop, cv2.COLOR_BGR2GRAY)

    crop = cv2.resize(crop, (150, 150))

    crop = crop / 255.0

    crop = np.expand_dims(crop, axis=-1)
    crop = np.expand_dims(crop, axis=0)

    crop = crop.astype(np.float32)

    return crop

# =========================
# WEBCAM
# =========================

cap = cv2.VideoCapture(0)

while True:

    ret, frame = cap.read()

    if not ret:
        break

    frame = cv2.flip(frame, 1)

    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    results = face_mesh.process(rgb)

    h, w = frame.shape[:2]

    if results.multi_face_landmarks:

        for face_landmarks in results.multi_face_landmarks:

            # =====================
            # EYE BOX
            # =====================

            eye_points = []

            for idx in LEFT_EYE + RIGHT_EYE:

                lm = face_landmarks.landmark[idx]

                x = int(lm.x * w)
                y = int(lm.y * h)

                eye_points.append((x, y))

            eye_x = [p[0] for p in eye_points]
            eye_y = [p[1] for p in eye_points]

            ex1 = min(eye_x) - 20
            ey1 = min(eye_y) - 20
            ex2 = max(eye_x) + 20
            ey2 = max(eye_y) + 20

            cv2.rectangle(
                frame,
                (ex1, ey1),
                (ex2, ey2),
                (0,255,0),
                2
            )

            eye_crop = preprocess_crop(
                frame,
                ex1, ey1,
                ex2, ey2
            )

            if eye_crop is not None:

                eye_interpreter.set_tensor(
                    eye_input[0]['index'],
                    eye_crop
                )

                eye_interpreter.invoke()

                eye_pred = eye_interpreter.get_tensor(
                    eye_output[0]['index']
                )

                eye_score = float(eye_pred[0][0])

                if eye_score > 0.5:
                    eye_text = "Eye Open"
                else:
                    eye_text = "Eye Closed"

                eye_acc = eye_score * 100

                cv2.putText(
                    frame,
                    f"{eye_text}: {eye_acc:.1f}%",
                    (ex1, ey1 - 10),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    0.7,
                    (0,255,0),
                    2
                )

            # =====================
            # MOUTH BOX
            # =====================

            mouth_points = []

            for idx in MOUTH:

                lm = face_landmarks.landmark[idx]

                x = int(lm.x * w)
                y = int(lm.y * h)

                mouth_points.append((x, y))

            mouth_x = [p[0] for p in mouth_points]
            mouth_y = [p[1] for p in mouth_points]

            mx1 = min(mouth_x) - 40
            my1 = min(mouth_y) - 40
            mx2 = max(mouth_x) + 40
            my2 = max(mouth_y) + 40

            cv2.rectangle(
                frame,
                (mx1, my1),
                (mx2, my2),
                (255,0,0),
                2
            )

            mouth_crop = preprocess_crop(
                frame,
                mx1, my1,
                mx2, my2
            )

            if mouth_crop is not None:

                mouth_interpreter.set_tensor(
                    mouth_input[0]['index'],
                    mouth_crop
                )

                mouth_interpreter.invoke()

                mouth_pred = mouth_interpreter.get_tensor(
                    mouth_output[0]['index']
                )

                mouth_score = float(mouth_pred[0][0])

                if mouth_score > 0.5:
                    mouth_text = "Yawning"
                else:
                    mouth_text = "Normal"

                mouth_acc = mouth_score * 100

                cv2.putText(
                    frame,
                    f"{mouth_text}: {mouth_acc:.1f}%",
                    (mx1, my1 - 10),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    0.7,
                    (255,0,0),
                    2
                )

    cv2.imshow(
        "Eye + Mouth Detection",
        frame
    )

    key = cv2.waitKey(1)

    if key == ord('q'):
        break

cap.release()

cv2.destroyAllWindows()
