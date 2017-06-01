package edu.polytechnique.smartrot.Model;

/**
 * MadgwickAHRS
 * Headtracker computation class.
 *
 * Code is based on Alexis Baskind libhedrot algorithm.
 * Part of code is derived from Sebastian Madgwick's open-source gradient descent angle estimation algorithm.
 *
 * Copyright 2017 Ecole Polytechnique
 * @author Tarek Marcé
 * @version 1.0
 */
class MadgwickAHRS {
    float SamplePeriod;
    private float Beta;
    private final float[] Quaternion;
    double MadgPitch, MadgRoll, MadgYaw;

    // angle estimation coefficients
    float betaMax = 2.5f, betaGain = 1.f;
    private float accLPtimeConstant = 0.01f; // lowpass filter time constant in seconds for the accel data. Default 10ms
    private float accLPalpha = 1 - (float) Math.exp(-SamplePeriod / accLPtimeConstant); // lowpass filter coefficient for the accel data (internal)

    //Reference quaternion
    private final float[] qref;
    private final float[] qcent;

    //References axis:
    private int orientation;

    //Rotation order
    private boolean pitchRollYaw;

    //Rotations direction inversion
    private boolean invertRotationDirection;

    MadgwickAHRS() {
        SamplePeriod = 0.01f;
        Beta = 0.041f;
        Quaternion = new float[]{1f, 0f, 0f, 0f};
        qref = new float[]{1f, 0f, 0f, 0f};
        qcent = new float[]{1f, 0f, 0f, 0f};
        orientation = 0;
        pitchRollYaw = false;
        invertRotationDirection = false;
    }

    private void quat_2_euler() {
        MadgYaw = Math.toDegrees(Math.atan2(2.0f * (qcent[1] * qcent[2] + qcent[3] * qcent[0]), 1.0f - 2.0f * (qcent[2] * qcent[2] + qcent[3] * qcent[3])));
        MadgPitch = Math.toDegrees(Math.asin(Math.min(Math.max(2.0f * (qcent[0] * qcent[2] - qcent[3] * qcent[1]), -1), 1)));
        MadgRoll = Math.toDegrees(Math.atan2(2.0f * (qcent[2] * qcent[3] + qcent[1] * qcent[0]), 1.0f - 2.0f * (qcent[1] * qcent[1] + qcent[2] * qcent[2])));
        if (pitchRollYaw) yawPitchRoll_2_pitchRollYaw();
        if (invertRotationDirection) inverseRotationDirection();
    }

    private void yawPitchRoll_2_pitchRollYaw() {
        double temp = MadgYaw;
        MadgYaw = MadgRoll;
        MadgRoll = temp;
    }

    private void inverseRotationDirection() {
        MadgYaw *= -1.f;
        MadgPitch *= -1.f;
        MadgRoll *= -1.f;
    }

    private void adjustAccLPalpha() {
        accLPalpha = 1 - (float) Math.exp(-SamplePeriod / accLPtimeConstant);
    }

    void centerAngles() {
        qref[0] = Quaternion[0];
        qref[1] = -Quaternion[1];
        qref[2] = -Quaternion[2];
        qref[3] = -Quaternion[3];
    }

    /**
     * Main function.
     * Computes orientation from data of the sensors.
     * Need to be called repeatedly.
     * @param gx Gyroscope's data on X axis.
     * @param gy Gyroscope's data on Y axis.
     * @param gz Gyroscope's data on Z axis.
     * @param ax Accelerometer's data on X axis.
     * @param ay Accelerometer's data on Y axis.
     * @param az Accelerometer's data on Z axis.
     * @param mx Magnetometer's data on X axis.
     * @param my Magnetometer's data on Y axis.
     * @param mz Magnetometer's data on Z axis.
     */
    void Update(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz) {
        // internal variables for the computation of the angles
        adjustAccLPalpha();
        float[] accLPstate = new float[3], accDataLP = new float[3];
        float q1 = Quaternion[0], q2 = Quaternion[1], q3 = Quaternion[2], q4 = Quaternion[3];   // short name local variable for readability
        float norm;
        float hx, hy, _2bx, _2bz;
        float s1, s2, s3, s4;
        float qDot1, qDot2, qDot3, qDot4;
        float a_norm2, m_norm2, gyro_norm2;

        // Auxiliary variables to avoid repeated arithmetic
        float _2q1mx;
        float _2q1my;
        float _2q1mz;
        float _2q2mx;
        float _4bx;
        float _4bz;
        float _2q1 = 2f * q1;
        float _2q2 = 2f * q2;
        float _2q3 = 2f * q3;
        float _2q4 = 2f * q4;
        float _2q1q3 = 2f * q1 * q3;
        float _2q3q4 = 2f * q3 * q4;
        float q1q1 = q1 * q1;
        float q1q2 = q1 * q2;
        float q1q3 = q1 * q3;
        float q1q4 = q1 * q4;
        float q2q2 = q2 * q2;
        float q2q3 = q2 * q3;
        float q2q4 = q2 * q4;
        float q3q3 = q3 * q3;
        float q3q4 = q3 * q4;
        float q4q4 = q4 * q4;

        // compute the squared norm of the gyro data => rough estimation of the movement
        gyro_norm2 = gx * gx
                + gy * gy
                + gz * gz;

        // low-pass the accelerometer data with a variable coefficient. If movement, alpha tends to 1 (no smoothing), if no movement, alpha tends to its min (smoothing)
        accDataLP[0] = accLPalpha * ax + (1 - accLPalpha) * accLPstate[0];
        accDataLP[1] = accLPalpha * ay + (1 - accLPalpha) * accLPstate[1];
        accDataLP[2] = accLPalpha * az + (1 - accLPalpha) * accLPstate[2];
        accLPstate[0] = accDataLP[0]; // filter state update
        accLPstate[1] = accDataLP[1]; // filter state update
        accLPstate[2] = accDataLP[2]; // filter state update


        // compute squared norms
        m_norm2 = mx * mx + my * my + mz * mz;
        a_norm2 = ax * ax + ay * ay + az * az;

        // return an error if magnetometer or accelerometer measurement invalid (avoids NaN in magnetometer normalisation)
        if (m_norm2 == 0.0 || a_norm2 == 0.0) return;

        // Normalise accelerometer measurement
        norm = 1 / (float) Math.sqrt(a_norm2);
        ax *= norm;
        ay *= norm;
        az *= norm;

        // Normalise magnetometer measurement
        norm = 1 / (float) Math.sqrt(m_norm2);
        mx *= norm;
        my *= norm;
        mz *= norm;

        // Reference direction of Earth's magnetic field
        _2q1mx = 2f * q1 * mx;
        _2q1my = 2f * q1 * my;
        _2q1mz = 2f * q1 * mz;
        _2q2mx = 2f * q2 * mx;
        hx = mx * q1q1 - _2q1my * q4 + _2q1mz * q3 + mx * q2q2 + _2q2 * my * q3 + _2q2 * mz * q4 - mx * q3q3 - mx * q4q4;
        hy = _2q1mx * q4 + my * q1q1 - _2q1mz * q2 + _2q2mx * q3 - my * q2q2 + my * q3q3 + _2q3 * mz * q4 - my * q4q4;
        _2bx = (float) Math.sqrt(hx * hx + hy * hy);
        _2bz = -_2q1mx * q3 + _2q1my * q2 + mz * q1q1 + _2q2mx * q4 - mz * q2q2 + _2q3 * my * q4 - mz * q3q3 + mz * q4q4;
        _4bx = 2f * _2bx;
        _4bz = 2f * _2bz;

        // Gradient decent algorithm corrective step
        s1 = -_2q3 * (2f * q2q4 - _2q1q3 - ax) + _2q2 * (2f * q1q2 + _2q3q4 - ay) - _2bz * q3 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q4 + _2bz * q2) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q3 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
        s2 = _2q4 * (2f * q2q4 - _2q1q3 - ax) + _2q1 * (2f * q1q2 + _2q3q4 - ay) - 4f * q2 * (1 - 2f * q2q2 - 2f * q3q3 - az) + _2bz * q4 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q3 + _2bz * q1) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q4 - _4bz * q2) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
        s3 = -_2q1 * (2f * q2q4 - _2q1q3 - ax) + _2q4 * (2f * q1q2 + _2q3q4 - ay) - 4f * q3 * (1 - 2f * q2q2 - 2f * q3q3 - az) + (-_4bx * q3 - _2bz * q1) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q2 + _2bz * q4) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q1 - _4bz * q3) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
        s4 = _2q2 * (2f * q2q4 - _2q1q3 - ax) + _2q3 * (2f * q1q2 + _2q3q4 - ay) + (-_4bx * q4 + _2bz * q2) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q1 + _2bz * q3) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q2 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
        norm = 1f / (float) Math.sqrt(s1 * s1 + s2 * s2 + s3 * s3 + s4 * s4);

        // normalise step magnitude
        s1 *= norm;
        s2 *= norm;
        s3 *= norm;
        s4 *= norm;

        // Apply feedback step
        // compute the dynamic parameter beta: no movement => beta maximum, lot of movement => beta tends to 0
        Beta = betaMax * (1 - Math.min(Math.max(betaGain * gyro_norm2, 0), 1));
        // Compute rate of change of quaternion
        qDot1 = 0.5f * (-q2 * gx - q3 * gy - q4 * gz) - Beta * s1;
        qDot2 = 0.5f * (q1 * gx + q3 * gz - q4 * gy) - Beta * s2;
        qDot3 = 0.5f * (q1 * gy - q2 * gz + q4 * gx) - Beta * s3;
        qDot4 = 0.5f * (q1 * gz + q2 * gy - q3 * gx) - Beta * s4;

        // Integrate to yield quaternion
        q1 += qDot1 * SamplePeriod;
        q2 += qDot2 * SamplePeriod;
        q3 += qDot3 * SamplePeriod;
        q4 += qDot4 * SamplePeriod;

        // Normalise quaternion
        norm = 1f / (float) Math.sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);
        Quaternion[0] = q1 * norm;
        Quaternion[1] = q2 * norm;
        Quaternion[2] = q3 * norm;
        Quaternion[3] = q4 * norm;

        quaternionComposition(qref, Quaternion, qcent);
        changeAxis(orientation);
        quat_2_euler();
    }

    /**
     * Change axis
     * By default, when the phone is flat on
     * the head of the user, screen up, and top of the
     * phone point to the right of the user,
     * axis are :
     * X points in front of the user
     * Y points to the right of the user
     * Z points above the user
     */
    private void changeAxis(int orientation) {
        float temp;
        switch (orientation) {
            // Standard
            // X:right, Y:front, Z:down
            case 0:
                temp = qcent[1];
                qcent[1] = qcent[2];
                qcent[2] = temp;
                qcent[3] *= -1.f;
                break;
            // X:right, Y:back, Z:above
            case 1:
                temp = qcent[1];
                qcent[1] = qcent[2];
                qcent[2] = -temp;
                qcent[3] = qcent[3];
                break;
            // X:down, Y:left, Z:above
            case 2:
                qcent[1] *= -1.f;
                qcent[2] *= -1.f;
                qcent[3] = qcent[3];
                break;
            default:
                break;
        }
    }

    private void quaternionComposition(float[] q0, float[] q1, float[] q2) {
        q2[0] = q0[0] * q1[0] - q0[1] * q1[1] - q0[2] * q1[2] - q0[3] * q1[3];
        q2[1] = q0[0] * q1[1] + q0[1] * q1[0] + q0[2] * q1[3] - q0[3] * q1[2];
        q2[2] = q0[0] * q1[2] - q0[1] * q1[3] + q0[2] * q1[0] + q0[3] * q1[1];
        q2[3] = q0[0] * q1[3] + q0[1] * q1[2] - q0[2] * q1[1] + q0[3] * q1[0];
    }

    /**
     * Accelerometer low-pass time constant.
     * Seconds values expected. Default is 0.01
     * Can be change by user in settings.
     * @param v New value (in seconds)
     */
    void setAccLPtimeConstant(float v) {
        accLPtimeConstant = v;
    }

    /**
     * Define axis refererences.
     * @param orientation 0, 1 or 2
     * @see Headtracker#setOrientation(int)
     */
    void setOrientation(int orientation) {
        this.orientation = orientation;
    }

    /**
     * Define rotations orders.
     * @param pitchRollYaw true = yaw pitch roll, false = roll pitch yaw
     * @see Headtracker#setRollPitchYaw(int)
     */
    void setRollPitchYaw(boolean pitchRollYaw) {
        this.pitchRollYaw = pitchRollYaw;
    }

    /**
     * Set the directions of the angles.
     * @param invertRotationDirection true = clockwise direction, false = counter clockwise.
     */
    void setInvertRotationDirection(boolean invertRotationDirection) {
        this.invertRotationDirection = invertRotationDirection;
    }
}
