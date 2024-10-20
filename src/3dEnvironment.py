import pybullet as pb
import numpy as np
import time
import csv
import plotly.graph_objects as go


# this works but performance was really bad at high velocities 


def read_csv_columns(file_path):
    with open(file_path, 'r') as csvfile:
        reader = csv.reader(csvfile)
        headers = next(reader)
        
        data = []
        for row in reader:
            converted_row = []
            for value in row:
                try:
                    converted_row.append(float(value))
                except ValueError:
                    converted_row.append(value)  # Keep original value if not convertible
            data.append(converted_row)
        
        # Transpose data to get columns
        columns = list(map(list, zip(*data)))
    
    return headers, columns



file_path = 'data.csv'
headers, columns = read_csv_columns(file_path)






# Create a figure with a line plot


fig = go.Figure(data=go.Scatter(x=columns[2], y=columns[3], mode='lines'))

# Add title and labels
fig.update_layout(
    title='Atmospheric effect',
    xaxis_title='Zposition',
    yaxis_title='Acceleration'
)

# Show the plot
fig.show()


#timestep = 0.0004  # Example: set to your desired time step value
#
## Compute acceleration (approximate derivative of velocity with respect to time)
## Assuming columns[3] is time, and columns[7] is velocity
#velocity_diff = np.diff(columns[7])  # Velocity differences
#
## Compute acceleration using the given timestep
#acceleration = velocity_diff / timestep
#
## Z-position needs to have one fewer value as well due to the derivative
#z_position = columns[3][1:]  # Corresponding Z-position values (excluding the first one)
#
## Create the plot
#fig = go.Figure(data=go.Scatter(x=z_position, y=acceleration, mode='lines'))
#
## Add title and labels
#fig.update_layout(
#    title='Acceleration vs Z-position',
#    xaxis_title='Z-position',
#    yaxis_title='Acceleration'
#)
#
## Show the plot
#fig.show()
#


def directionVectorToEuler(vector):
    x, y, z = vector
    pitch = np.arcsin(-y)
    yaw = np.arctan2(x, z)
    roll = 0
    
    return pitch, yaw, roll



#physicsClient = pb.connect(pb.GUI)

cylinderStartPos = [columns[1][0], columns[2][0], columns[3][0]]
cylinderStartOrientation = pb.getQuaternionFromEuler(directionVectorToEuler([columns[4][0], columns[5][0], columns[6][0]]))



cylinderVisualShapeId = pb.createVisualShape(pb.GEOM_MESH, fileName="../3d/falcon.obj")
#cylinderVisualShapeId = pb.createVisualShape(pb.GEOM_CYLINDER, radius=3.6/2, length=48)

cylinderId = pb.createMultiBody(baseVisualShapeIndex=cylinderVisualShapeId,
                                basePosition=cylinderStartPos, 
                                baseOrientation=cylinderStartOrientation)




#cylinderVisualShapeId = pb.createVisualShape(pb.GEOM_CYLINDER, radius=.3, length=40)

#cylinderId2 = pb.createMultiBody(baseVisualShapeIndex=cylinderVisualShapeId,
#                                basePosition=cylinderStartPos, 
#                                baseOrientation=cylinderStartOrientation)


#cameraDistance = 40
#cameraYaw = 50
#cameraPitch = -60
#
#pb.setRealTimeSimulation(1)
#
#start_time = time.time()

# Run the simulation for a certain time
count = 0


#while True:
#    
#    elapsed_time = time.time() - start_time  # Real-time clock (seconds)
#    
#    while(elapsed_time > columns[0][count]):
#        count = count + 1
#
#    new_cylinder_pos = [columns[1][count], columns[2][count],columns[3][count]]  
#    newCylinderOrientation = pb.getQuaternionFromEuler(directionVectorToEuler([columns[4][count], columns[5][count], columns[6][count]]))
#
#    pb.resetBasePositionAndOrientation(cylinderId, new_cylinder_pos, newCylinderOrientation)
#
#
#    #new_cylinder_pos = [columns[1][count], columns[2][count],columns[3][count]]  
#    #newCylinderOrientation = pb.getQuaternionFromEuler(directionVectorToEuler([columns[7][count], columns[8][count], columns[9][count]]))
#    
#
#    #pb.resetBasePositionAndOrientation(cylinderId2, new_cylinder_pos, newCylinderOrientation)
#    # Get the current position of the cylinder
#    cylinderPos, _ = pb.getBasePositionAndOrientation(cylinderId)
#
#    
#    
#    pb.resetDebugVisualizerCamera(cameraDistance, cameraYaw, cameraPitch, cylinderPos)
#    
#
#    time.sleep(1/30)






