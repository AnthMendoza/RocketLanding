import * as THREE from "three";
import { GLTFLoader } from "three/addons/loaders/GLTFLoader.js";
import { OBJLoader } from "three/addons/loaders/OBJLoader.js";

let startTime = null; // Declare startTime here

const staticDuration = 0; 
const speed = 1;




  // Create the scene
  const scene = new THREE.Scene();
  scene.background = new THREE.Color(0x000000); // Light gray background 0xaaaaaa

  // Set up the camera
  const simulationViewPort = document.getElementById('simulationViewPort');
  const canvas = document.getElementById('threeCanvas');
  const camera = new THREE.PerspectiveCamera(96, window.innerWidth / window.innerHeight, 0.001, 100000);
  camera.aspect = simulationViewPort.clientWidth / simulationViewPort.clientHeight;
  const cameraOffset = new THREE.Vector3(30, 50, 10);  // Fixed offset relative to the object


  // Create the renderer
  const renderer = new THREE.WebGLRenderer({ canvas: canvas });
  renderer.setSize(simulationViewPort.clientWidth, simulationViewPort.clientHeight);
  camera.updateProjectionMatrix();
  

  // Add lighting to the scene
  const ambientLight = new THREE.AmbientLight(0x404040); // Soft light
  scene.add(ambientLight);

  const directionalLight = new THREE.DirectionalLight(0xffffff, 1.0); // Increase intensity
  directionalLight.position.set(1, 1, 1).normalize();
  scene.add(directionalLight);



const floorGeometry = new THREE.PlaneGeometry(30, 30); 

// Create a green material
const floorMaterial = new THREE.MeshStandardMaterial({
    color: 0x00ff00,
    side: THREE.DoubleSide, 
    roughness: 0.8, 
    metalness: 0.2 
});


const floor = new THREE.Mesh(floorGeometry, floorMaterial);
floor.rotation.x = -Math.PI / 2;
floor.position.y = -26;
floor.position.x = data.Xposition[data.Xposition.length - 2 ];
floor.position.z = data.Yposition[data.Yposition.length - 2 ];
floor.receiveShadow = true;
scene.add(floor);





  // Variables to hold the object once loaded
  let object;

  // Load the OBJ model
  const loader = new OBJLoader();
  const GLTFloader = new GLTFLoader();
  loader.load(
    'https://raw.githubusercontent.com/AnthMendoza/RocketLanding/refs/heads/main/src/static/3d/falcon.obj',  // Replace with your .obj file path
    function (loadedObject) {
      object = loadedObject;
      object.position.y = 1; // Start position of the object
      object.traverse(function (child) {
        if (child.isMesh) {
          child.material = new THREE.MeshStandardMaterial({ color: 0x637ea8 }); // Set a default color
        }
      });
      scene.add(object);
    },
    function (xhr) {
      console.log((xhr.loaded / xhr.total * 100) + '% loaded');
    },
    function (error) {
      console.log('An error occurred while loading the OBJ file:', error);
    }
  );

const vertexShader = document.getElementById("vertex-shader").textContent;
const fragShader = document.getElementById("fragment-shader").textContent;

  const getMaterial = (color) =>
    new THREE.ShaderMaterial({
        fragmentShader: fragShader,
        vertexShader: vertexShader,
        glslVersion: THREE.GLSL3,
        uniforms: {
            uAnimationProgress: { value: (Date.now() / 2000) % 1 },
            uColor: { value: color },
        },
        transparent: true,
    });

const material1 = getMaterial([1, 0, 0]);
const material2 = getMaterial([1, 1, 0]);

let mesh1;
let mesh2;
let pivot1 = new THREE.Object3D();
let pivot2 = new THREE.Object3D();
  GLTFloader.load(
    "../static/3d/fire-template.glb",
    function (gltf) {
        const { geometry } = gltf.scene.children[0];
        mesh1 = new THREE.Mesh(geometry, material1);

        mesh2 = new THREE.Mesh(geometry, material2);
        const clock = new THREE.Clock();

        function render() {
            const elapsedTime = clock.getElapsedTime();
            material1.uniforms.uAnimationProgress.value = (elapsedTime *1) % 1;
            
            material2.uniforms.uAnimationProgress.value =(elapsedTime *1 + 0.2) % 1;

            material1.side = THREE.BackSide;
            material2.side = THREE.BackSide;
            mesh1.renderOrder = 0;
            mesh2.renderOrder = 1;

            material2.side = THREE.FrontSide;
            material1.side = THREE.FrontSide;
            mesh1.renderOrder = 1;
            mesh2.renderOrder = 0;
            
            renderer.render(scene, camera);
            window.requestAnimationFrame(render);

        }
        pivot1.add(mesh1);
        pivot2.add(mesh2);
        mesh1.position.set(0, 1, 0);
        mesh2.position.set(0, 1, 0);
        mesh1.visible = false;
        mesh2.visible = false;
        scene.add(pivot1);
        scene.add(pivot2);


        render();
    },
    undefined,
    console.error
);

console.log(data.engineVector0);
console.log(data.engineVector1);
console.log(data.engineVector2);
    
let  currentX = 0;
let  currentZ = 0;
let  currentY = 0;

  let count = 1;

  function updateObjectByTime(currentTime) {
    if (!object) return;  // Wait until the object is loaded

    while(count < data.VectorTimeStamp.length -2 && currentTime > parseFloat(data.VectorTimeStamp[count])){
      count +=1;
    }
    currentX = parseFloat(data.Xposition[count]);
    currentZ = parseFloat(data.Zposition[count]);
    currentY =  parseFloat(data.Yposition[count]);
    // Ensure count doesn't exceed bounds
    // Set the object's position and rotation
    object.position.set(currentX , currentZ , currentY);
    let directionVector = new THREE.Vector3(
      parseFloat(data.vehicleState0[count]),  
      parseFloat(data.vehicleState2[count]),  
      parseFloat(data.vehicleState1[count])
    );  

    const isVisible = parseFloat(data.enginePower[count]) !== 0;
    mesh1.visible = isVisible;
    mesh2.visible = isVisible;

    pivot1.scale.set( 4 * parseFloat(data.enginePower[count]) + 4+ .5 , 15 * parseFloat(data.enginePower[count]) + 18 + 1 , 4 * parseFloat(data.enginePower[count]) + 4+ .5);
    pivot2.scale.set( 4 * parseFloat(data.enginePower[count]) + 4 , 15 * parseFloat(data.enginePower[count]) + 18 , 4 * parseFloat(data.enginePower[count]) + 4);
    let distance = -17;

    if(parseFloat(data.velocity[count]) > 400 ){
      distance = -20
    }
    if(count >= data.VectorTimeStamp.length - 2){
      mesh1.visible = false;
      mesh2.visible = false;
    }
    let stateVectorX = parseFloat(data.vehicleState0[count]);
    let stateVectorY = parseFloat(data.vehicleState1[count]);
    let stateVectorZ = parseFloat(data.vehicleState2[count]);
    let normalVector  = Math.sqrt(stateVectorX * stateVectorX + stateVectorY * stateVectorY + stateVectorZ * stateVectorZ );
    let adjustedX = currentX + stateVectorX * normalVector * distance;
    let adjustedY = currentY + stateVectorY * normalVector * distance;
    let adjustedZ = currentZ + stateVectorZ * normalVector * distance;

    pivot1.position.set(adjustedX , adjustedZ , adjustedY);
    pivot2.position.set(adjustedX , adjustedZ , adjustedY);
    pivot1.lookAt(adjustedX + parseFloat(data.engineVector0[count]) , adjustedZ + parseFloat(data.engineVector2[count]) , adjustedY+ parseFloat(data.engineVector1[count]));
    pivot2.lookAt(adjustedX + parseFloat(data.engineVector0[count]) , adjustedZ + parseFloat(data.engineVector2[count]) , adjustedY+ parseFloat(data.engineVector1[count]));
    pivot1.rotateX(Math.PI * 3 / 2); 
    pivot2.rotateX(Math.PI * 3 / 2);

    if(parseFloat(data.velocity[count]) > 400 ){
      pivot1.rotateX(Math.PI); 
      pivot2.rotateX(Math.PI );
      pivot1.scale.set(15,10,15)
      pivot2.scale.set(14,10,14);

    }




    const targetPosition = object.position.clone().add(directionVector);
    object.lookAt(targetPosition); 

    // Update camera to follow object with a static offset
    camera.position.copy(object.position).add(cameraOffset);
    camera.lookAt(object.position);

    const overlayText = document.getElementById('overlay-text');//+parseFloat(data.Zposition[0])).toFixed(1)
    overlayText.innerHTML = `Position( meters ):<br>X=${object.position.x.toFixed(1)}
                                                <br>Y=${(-object.position.z.toFixed(1))}
                                                <br>altitude=${object.position.y.toFixed(1)} <br>
                                                <br> Velocity( m/s , mph )<br>${parseFloat(data.velocity[count]).toFixed(3)}
                                                <br>${(parseFloat(data.velocity[count])*2.237).toFixed(3)}<br>
                                                <br> Acceleration( G )${parseFloat(data.gForce[count]).toFixed(3)}`;
  }
  



  // Animation loop
  function animate(timestamp) {
    requestAnimationFrame(animate);

    
    if (startTime === null) startTime = timestamp;
    const elapsedTime = (timestamp - startTime) / 1000;  // time in seconds

    if (elapsedTime >= staticDuration) {
      updateObjectByTime(elapsedTime - staticDuration); // Subtract staticDuration from elapsed time
    }
    renderer.render(scene, camera);

  }
  animate(startTime);

  // Handle window resizing
  window.addEventListener('resize', () => {
    const simulationViewPort = document.getElementById('simulationViewPort');
    renderer.setSize(simulationViewPort.clientWidth, simulationViewPort.clientHeight);
    camera.aspect = simulationViewPort.clientWidth / simulationViewPort.clientHeight;
    camera.updateProjectionMatrix();
  });